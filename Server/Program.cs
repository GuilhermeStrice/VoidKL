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
        static volatile bool stop = false;

        static void HandleClient(TcpClient client)
        {
            FileStream file = null;
            NetworkStream stream = client.GetStream();

            try
            {
                byte[] user_buffer = new byte[1024];
                int user_recv = stream.Read(user_buffer, 0, 1024);

                string username = Encoding.ASCII.GetString(user_buffer, 0, user_recv - 1) + ".txt";

                file = File.Open(Directory.GetCurrentDirectory() + "\\" + username, FileMode.OpenOrCreate, FileAccess.ReadWrite);

                byte[] data = Encoding.ASCII.GetBytes(Environment.NewLine + DateTime.Now.ToString() + Environment.NewLine);
                file.Write(data, 0, data.Length);

                byte[] start_message = Encoding.ASCII.GetBytes("start");
                stream.Write(start_message, 0, start_message.Length);
            }
            catch (Exception)
            {
                try
                {
                    file.Flush();
                    file.Close();
                    client.Close();
                }
                catch { }

                file = null;
            }

            while (!stop)
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
                        //Console.WriteLine(Encoding.ASCII.GetString(buffer, 0, recv));
                    }
                }
                catch
                {
                    break;
                }
            }

            if (file != null)
            {
                file.Flush();
                file.Close();
            }
        }

        static void Main(string[] args)
        {
            TcpListener server = new TcpListener(IPAddress.Any, 39852);

            server.Start();

            new Thread(() =>
            {
                while (!stop)
                {
                    TcpClient client = server.AcceptTcpClient();

                    new Thread(() => HandleClient(client)).Start();
                }
            });

            while (true)
            {
                string command = Console.ReadLine();

                if (command == "stop")
                {
                    stop = true;
                }
            }
        }
    }
}
