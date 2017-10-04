using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Net;

namespace Server
{
    class Program
    {
        static void Main(string[] args)
        {
            TcpListener server = new TcpListener(IPAddress.Any, 39852);

            server.Start();

            while (true)
            {
                TcpClient client = server.AcceptTcpClient();

                new Thread(() =>
                {
                    FileStream file = null;
                    NetworkStream stream = client.GetStream();
                    byte[] user_buffer = new byte[1024];
                    int user_recv = stream.Read(user_buffer, 0, 1024);

                    string username = Encoding.ASCII.GetString(user_buffer, 0, user_recv - 1) + ".txt";

                    if (!File.Exists(Directory.GetCurrentDirectory() + "\\" + username))
                        File.Create(Directory.GetCurrentDirectory() + "\\" + username).Close();

                    file = File.OpenWrite(Directory.GetCurrentDirectory() + "\\" + username);

                    byte[] data = Encoding.ASCII.GetBytes(Environment.NewLine + DateTime.Now.ToString() + Environment.NewLine);
                    file.Write(data, 0, data.Length);

                    byte[] start_message = Encoding.ASCII.GetBytes("start");
                    stream.Write(start_message, 0, start_message.Length);

                    while (true)
                    {
                        try // idk if this will work but it will do the trick
                        {
                            byte[] buffer = new byte[4096];
                            int recv = stream.Read(buffer, 0, 4096);

                            if (recv > 0)
                            {
                                string msg = Encoding.ASCII.GetString(buffer, 0, recv);

                                if (msg == "disconnect")
                                {
                                    break;
                                }

                                file.Write(buffer, 0, recv);
                                Console.WriteLine(Encoding.ASCII.GetString(buffer, 0, recv));
                            }
                        }
                        catch
                        {
                            break;
                        }
                    }

                    file.Flush();
                    file.Close();
                });
            }
        }
    }
}
