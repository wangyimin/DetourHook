using System;
using System.Windows.Forms;
using System.IO;
namespace Tester
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            File.WriteAllText("D:\\Wang\\Test.txt", "Hello world");
            MessageBox.Show("Hello world!");
        }
    }
}
