namespace TimeTagger
{
    partial class setup
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(setup));
            this.bufsize = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.ttnum = new System.Windows.Forms.NumericUpDown();
            this.startbtn = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.bufsize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ttnum)).BeginInit();
            this.SuspendLayout();
            // 
            // bufsize
            // 
            this.bufsize.Location = new System.Drawing.Point(79, 2);
            this.bufsize.Maximum = new decimal(new int[] {
            1215752191,
            23,
            0,
            0});
            this.bufsize.Minimum = new decimal(new int[] {
            10000000,
            0,
            0,
            0});
            this.bufsize.Name = "bufsize";
            this.bufsize.Size = new System.Drawing.Size(120, 20);
            this.bufsize.TabIndex = 2;
            this.bufsize.Value = new decimal(new int[] {
            50000000,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Buffer Size:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 35);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(110, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Time Tagger Number:";
            // 
            // ttnum
            // 
            this.ttnum.Location = new System.Drawing.Point(128, 28);
            this.ttnum.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.ttnum.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.ttnum.Name = "ttnum";
            this.ttnum.Size = new System.Drawing.Size(71, 20);
            this.ttnum.TabIndex = 3;
            this.ttnum.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // startbtn
            // 
            this.startbtn.Location = new System.Drawing.Point(38, 57);
            this.startbtn.Name = "startbtn";
            this.startbtn.Size = new System.Drawing.Size(130, 30);
            this.startbtn.TabIndex = 1;
            this.startbtn.Text = "Start";
            this.startbtn.UseVisualStyleBackColor = true;
            this.startbtn.Click += new System.EventHandler(this.startbtn_Click);
            // 
            // setup
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(205, 92);
            this.Controls.Add(this.startbtn);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.ttnum);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.bufsize);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "setup";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Choose Time Tagger";
            ((System.ComponentModel.ISupportInitialize)(this.bufsize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ttnum)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown bufsize;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown ttnum;
        private System.Windows.Forms.Button startbtn;
    }
}

