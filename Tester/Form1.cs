using System;
using System.Windows.Forms;
using System.IO;
using System.IO.MemoryMappedFiles;
namespace Tester
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            char[] v = {'A', 'B', 'C'};
            MemoryMappedFile mem = MemoryMappedFile.CreateNew("TCSC", 1024);
            {
                using (MemoryMappedViewAccessor accessor = mem.CreateViewAccessor())
                {
                    accessor.WriteArray(0, v, 0, v.Length);
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            using (MemoryMappedFile share_mem = MemoryMappedFile.OpenExisting("TCSC"))
            {
                using (MemoryMappedViewAccessor accessor = share_mem.CreateViewAccessor())
                {
                    char[] data = new char[128];
                    accessor.ReadArray(0, data, 0, data.Length);
                }
            }
            File.WriteAllText("D:\\Wang\\Test.txt", "Hello world");
            MessageBox.Show("Hello world!");
        }
    }
}
