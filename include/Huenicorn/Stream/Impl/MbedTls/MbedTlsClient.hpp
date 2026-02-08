#pragma once

#include <Huenicorn/Stream/IDtlsClient.hpp>

#include <string>
#include <vector>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/timing.h>


namespace Huenicorn::Stream
{
  /**
   * @brief Wrapper around Mbed-TLS library to provide TLS v1.2 connection to Hue bridge
   * 
   */
  class MbedTlsClient : public IDtlsClient
  {
    template <auto FreeFunc>
    struct MbedTlsDeleter
    {
      template <typename T>
      void operator()(T* ptr) const noexcept
      {
        if(ptr){
          FreeFunc(ptr);
          ptr = nullptr;
        }
      }
    };

    template <typename T, auto FreeFunc>
    using MbedTlsUniquePtr = std::unique_ptr<T, MbedTlsDeleter<FreeFunc>>;

    using UniqueNetContext = MbedTlsUniquePtr<mbedtls_net_context, mbedtls_net_free>;
    using UniqueCert = MbedTlsUniquePtr<mbedtls_x509_crt, mbedtls_x509_crt_free>;
    using UniqueSsl = MbedTlsUniquePtr<mbedtls_ssl_context, mbedtls_ssl_free>;
    using UniqueSslConfig = MbedTlsUniquePtr<mbedtls_ssl_config, mbedtls_ssl_config_free>;
    using UniqueEntropy = MbedTlsUniquePtr<mbedtls_entropy_context, mbedtls_entropy_free>;
    using UniqueCtrDrbg = MbedTlsUniquePtr<mbedtls_ctr_drbg_context, mbedtls_ctr_drbg_free>;

    // Constants
    static const std::string Hostname;
    static const unsigned HandshakeAttempts;

  public:
    // Constructor / destructor
    /**
     * @brief MbedTlsClient constructor
     * 
     * @param dtlsConfig DTLS configuration
     */
    explicit MbedTlsClient(const DtlsConfig& dtlsConfig);

    virtual bool isConnected() const override;

    /**
     * @brief MbedTlsClient destructor
     * 
     */
    ~MbedTlsClient();

    /**
     * @brief Calls all the inner initializations
     * 
     */
    virtual void init() override;

    virtual void shutdown() override;


    /**
     * @brief Sends byte buffer to the server
     * 
     * @param requestBuffer Byte buffer to send to the server
     * @return true Sending was successful
     * @return false Sending failed
     */
    virtual bool send(
      std::span<const std::byte> requestBuffer
    ) override;

  private:
    // Private methods
    /**
     * @brief Initializes MberdTLS objects
     * 
     */
    void _initMembers();


    /**
     * @brief Initializes debug messages for Mbed-TLS
     * 
     */
    void _initDebug();


    /**
     * @brief Initializes the random number generator
     * 
     */
    void _initRNG();


    /**
     * @brief Initializes the connection
     * 
     */
    void _initConnection();


    /**
     * @brief Initializes the SSL layer with PSK
     * 
     */
    void _initSSL();


    /**
     * @brief Execute handshake
     * 
     */
    void _handshake();


    /**
     * @brief Free all the allocated MbedTLS members
     * 
     */
    void _deallocate();

    // Attributes
    DtlsConfig m_dtlsConfig;

    std::vector<int> m_ciphers;
    bool m_isConnected{false};

    // MbedTLS
    UniqueNetContext m_serverFd;
    UniqueEntropy m_entropy;
    UniqueCtrDrbg m_ctrDrbg;
    UniqueSsl m_ssl;
    UniqueSslConfig m_conf;
    UniqueCert m_cacert;
    mbedtls_timing_delay_context m_timer;
  };
}
