using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TimeTagger
{
    public partial class setup : Form
    {
        public setup()
        {
            InitializeComponent();
        }
        private void startbtn_Click(object sender, EventArgs e)
        {
            Form f = new main((int)ttnum.Value, (ulong)bufsize.Value);
            this.Hide();
            f.ShowDialog();
            Application.Exit();
        }
    }
}
