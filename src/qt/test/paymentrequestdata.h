// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_TEST_PAYMENTREQUESTDATA_H
#define BITCOIN_QT_TEST_PAYMENTREQUESTDATA_H

//
// Data for paymentservertests.cpp
//

// Base64/DER-encoded fake certificate authority certificates.
// Convert pem to base64/der with:
// openssl x509 -in cert.pem -inform PEM -outform DER | openssl enc -base64

// Serial Number: 10302349811211485352 (0x8ef94c91b112c0a8)
// Issuer: CN=PaymentRequest Test CA
// Subject: CN=PaymentRequest Test CA
// Not Valid After : Dec  8 16:37:24 2022 GMT
//
const char* caCert1_BASE64 = "";

// Serial Number: f0:da:97:e4:38:d7:64:16
// Issuer: CN=PaymentRequest Test CA
// Subject: CN=PaymentRequest Test CA
// Not Valid After : Jan  8 18:21:06 2025 GMT
//
const char* caCert2_BASE64 = "";

//
// This payment request validates directly against the
// caCert1 certificate authority.
//
const char* paymentrequest1_cert1_BASE64 = "";

//
// Signed, but expired, merchant cert in the request
//
const char* paymentrequest2_cert1_BASE64 = "";

//
// 10-long certificate chain, all intermediates valid
//
const char* paymentrequest3_cert1_BASE64 = "";

//
// Long certificate chain, with an expired certificate in the middle
//
const char* paymentrequest4_cert1_BASE64 = "";

//
// Validly signed, but by a CA not in our root CA list
//
const char* paymentrequest5_cert1_BASE64 = "";

//
// Contains a testnet paytoaddress, so payment request network doesn't match client network
//
const char* paymentrequest1_cert2_BASE64 = "";

//
// Expired payment request (expires is set to 1 = 1970-01-01 00:00:01)
//
const char* paymentrequest2_cert2_BASE64 = "";

//
// Unexpired payment request (expires is set to 0x7FFFFFFFFFFFFFFF = max. int64_t)
//
const char* paymentrequest3_cert2_BASE64 = "";

//
// Unexpired payment request (expires is set to 0x8000000000000000 > max. int64_t, allowed uint64)
//
const char* paymentrequest4_cert2_BASE64 = "";

//
// Payment request with amount overflow (amount is set to 107822406.26 ZHC)
//
const char* paymentrequest5_cert2_BASE64 = "";

#endif // BITCOIN_QT_TEST_PAYMENTREQUESTDATA_H
