using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Diagnostics;
using System.Net;

namespace Server
{
    class Program
    {
        static volatile bool stop = false;

        static void HandleClient(TcpClient client)
        {
            Stopwatch stopwatch = new Stopwatch();
            FileStream file = null;
            NetworkStream stream = client.GetStream();
            stream.ReadTimeout = 900000;
            stream.WriteTimeout = 900000;

            /*try
            {
                byte[] user_buffer = new byte[1024];
                int user_recv = stream.Read(user_buffer, 0, 1024);
                
                string path = Directory.GetCurrentDirectory() + "\\" + Encoding.ASCII.GetString(user_buffer, 0, user_recv - 1) + ".txt";

                if (File.Exists(path))
                    file = File.Open(path, FileMode.Open, FileAccess.ReadWrite);
                else
                {
                    file = File.Create(path);
                }

                byte[] data = Encoding.ASCII.GetBytes(Environment.NewLine + DateTime.Now.ToString() + Environment.NewLine);
                file.Write(data, 0, data.Length);
            }
            catch (Exception e)
            {
                try
                {
                    file.Flush();
                    file.Close();
                    client.Close();
                }
                catch { }

                file = null;
            }*/

            stopwatch.Start();
            while (!stop)
            {
                try
                {
                    if (stream.DataAvailable)
                    {
                        stopwatch.Reset();
                        stopwatch.Start();
                        byte[] receive = new byte[4096];
                        int recv = stream.Read(receive, 0, 4096);

                        if (recv > 0)
                        {
                            //string msg = Encoding.ASCII.GetString(receive, 0, recv);
                            //file.Write(receive, 0, recv);
                            Console.WriteLine(Encoding.ASCII.GetString(receive, 0, recv));
                        }
                    }
                    else if (stopwatch.ElapsedMilliseconds == 900000)
                        break;
                }
                catch
                {
                    break;
                }
            }

            /*if (file != null)
            {
                file.Flush();
                file.Close();
                client.Close();
            }*/

            Console.WriteLine("Client Disconnected");
        }

        static void Main(string[] args)
        {
            TcpListener server = new TcpListener(IPAddress.Any, 3000);

            server.Start();
            
            while (!stop)
            {
                TcpClient client = server.AcceptTcpClient();

                new Thread(() => HandleClient(client)).Start();
                Console.WriteLine("Client Connected");
            }
        }
    }
}
