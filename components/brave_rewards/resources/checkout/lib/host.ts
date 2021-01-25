/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { OrderInfo, Host, HostListener } from './interfaces'

function parseDialogArgs (): OrderInfo {
  const argString = chrome.getVariableValue('dialogArguments')

  let args: any
  try {
    args = JSON.parse(argString)
  } catch {
    // Ignore
  }

  const { orderInfo } = Object(args)
  // TODO(zenparsing): Throw if orderInfo is invalid?

  return {
    description: orderInfo.description,
    total: orderInfo.total
  }
}

interface BalanceDetails {
  total: number
  rates: Record<string, number>
}

interface RateDetails {
  rate: number
  lastUpdated: string
}

/*interface ExternalWalletDetails {
  status: number
}*/

export function createHost (): Host {
  let hostListener: HostListener | null = null
  let balanceDetails: BalanceDetails | null = null
  let rateDetails: RateDetails | null = null
  // let externalWalletDetails: ExternalWalletDetails | null = null
  let orderInfo = parseDialogArgs()

  // TODO(zenparsing): Is this required?
  self.i18nTemplate.process(document, self.loadTimeData)

  function sendWalletInfo () {
    if (hostListener && balanceDetails) {
      const balance = balanceDetails.total
      // TODO(zenparsing): Any other status codes?
      // const verified = externalWalletDetails.status === 1
      const verified = 1 === 1
      hostListener.onWalletUpdated({ balance, verified })
    }
  }

  function sendRateInfo() {
    if (hostListener && rateDetails) {
      const rate = rateDetails.rate
      const lastUpdated = new Date(Number(rateDetails.lastUpdated)).toISOString()
      hostListener.onRatesUpdated({rate, lastUpdated})
    }
  }

  self.cr.addWebUIListener('walletBalanceUpdated', (event: any) => {
    const { balance } = event

    console.log("Wallet balance updated")
    console.log(event)

    // TODO(zenparsing): Details can be empty if rewards
    // are not enabled and the user does not have a wallet.
    // How do we detect this case without ignoring a real
    // error on startup? If we don't have any details, then
    // how do we get the rates which are required for
    // credit card processing?
    if (!balance) {
      return
    }

    balanceDetails = balance as BalanceDetails

    console.log("Balance Details")
    console.log(balanceDetails)

    sendWalletInfo()
  })

  self.cr.addWebUIListener('rewardsParametersUpdated', (event: any) => {
    console.log("Rewards params updated")
    console.log(event)
    if (!event) {
      return
    }
    rateDetails = event as RateDetails
    console.log("Rate Details")
    console.log(rateDetails)
    sendRateInfo()
  })

  /*self.cr.addWebUIListener('externalWalletUpdated', (event: any) => {
    console.log("On external wallet updated")
    const { details } = event
    if (!details) {
      return
    }
    externalWalletDetails = details as ExternalWalletDetails
    sendWalletInfo()
  })*/

  console.log("Getting wallet balance")
  chrome.send('getWalletBalance')
  chrome.send('getRewardsParameters')

  return {
    getLocaleString (key: string) {
      return self.loadTimeData.getString(key)
    },

    closeDialog () {
      chrome.send('dialogClose', [JSON.stringify({'action': 'cancel'})])
    },

    payWithCreditCard (...args) {
      console.log('payWithCreditCard', ...args)
      // TODO(zenparsing): Send update to service
    },

    payWithWallet (...args) {
      console.log('payWithWallet', ...args)
      chrome.send('paymentRequestComplete')
      chrome.send('dialogClose')
      // TODO(zenparsing): Send update to service
    },

    setListener (listener) {
      hostListener = listener

      chrome.send('getWalletBalance')
      chrome.send('getExternalWallet')

      queueMicrotask(() => {
        if (hostListener) {
          hostListener.onOrderUpdated(orderInfo)
        }
      })

      return () => { hostListener = null }
    }

  }
}
