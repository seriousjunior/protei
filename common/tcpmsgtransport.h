#ifndef TCPMSGTRANSPORT_H
#define TCPMSGTRANSPORT_H



#include <string>
#include <list>

class CTcpMsgStore
{
    int m_headerLength;
    int m_length; //!< Количество принятых байт
    int m_expected; //!< Ожидаемое количество байт
    std::string m_message; //!< Данные принятых пакетов

public:
    CTcpMsgStore();

    /** \brief Функция добавляет к сообщению новый блок данных
     * \param data [in] - данные
     * \param size [in] - количество байт
     * \return 0 в случае успеха, иначе < 0
     * */
    int addPacket(const char *data, int size);

    /** \brief Функция формирует возвращает принятое сообщение
     * \details Возвращаемое значение - размер сообщения без контрольной суммы.
     * \param message [in] - контейнер для сообщения
     * \return количество байт в сообщении в случае успеха, иначе < 0
     * */
    int getMessage(std::string &message);

    bool isMsgReady() const;

    void clear();
};

int sendTcpMessage(int sock, const std::string &message);

#endif // TCPMSGTRANSPORT_H
