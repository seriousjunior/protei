#ifndef CUDPMSGTRANSPORT_H
#define CUDPMSGTRANSPORT_H

#include <netinet/in.h>
#include <list>
#include <string>

class CUdpMsgStore
{
    int m_length; //!< Количество принятых байт
    std::list< std::string > m_recvPacks; //!< Данные принятых пакетов

public:
    CUdpMsgStore();

    /** \brief Функция добавляет к сообщению новый блок данных
     * \param data [in] - данные
     * \param size [in] - количество байт
     * \return 0 в случае успеха, иначе < 0
     * */
    int addPacket(const char *data, int size);

    /** \brief Функция формирует возвращает принятое сообщение
     * \details В строку помещается все сообщение, с контрольной суммой.
     * \details Возвращаемое значение - размер сообщения без контрольной суммы.
     * \param message [in] - контейнер для сообщения
     * \return количество байт в сообщении в случае успеха, иначе < 0
     * */
    int getMessage(std::string &message);

};

int sendUdpMessage(int sock, const struct sockaddr_in &addr, const std::string &message);

#endif // CUDPMSGTRANSPORT_H
