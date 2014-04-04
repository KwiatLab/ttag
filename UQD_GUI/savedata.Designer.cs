namespace TimeTagger
{
    partial class savedata
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(savedata));
            this.progress = new System.Windows.Forms.ProgressBar();
            this.stat = new System.Windows.Forms.Label();
            this.writetype = new System.Windows.Forms.GroupBox();
            this.btn = new System.Windows.Forms.Button();
            this.tagnumber = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.mostrecent = new System.Windows.Forms.RadioButton();
            this.entirebuffer = new System.Windows.Forms.RadioButton();
            this.filechooser = new System.Windows.Forms.SaveFileDialog();
            this.filewriter = new System.ComponentModel.BackgroundWorker();
            this.writetype.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tagnumber)).BeginInit();
            this.SuspendLayout();
            // 
            // progress
            // 
            this.progress.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.progress.Location = new System.Drawing.Point(0, 92);
            this.progress.Name = "progress";
            this.progress.Size = new System.Drawing.Size(239, 23);
            this.progress.TabIndex = 0;
            // 
            // stat
            // 
            this.stat.AutoSize = true;
            this.stat.Location = new System.Drawing.Point(12, 76);
            this.stat.Name = "stat";
            this.stat.Size = new System.Drawing.Size(179, 13);
            this.stat.TabIndex = 1;
            this.stat.Text = "Do NOT start runner while writing file";
            // 
            // writetype
            // 
            this.writetype.Controls.Add(this.btn);
            this.writetype.Controls.Add(this.tagnumber);
            this.writetype.Controls.Add(this.label1);
            this.writetype.Controls.Add(this.mostrecent);
            this.writetype.Controls.Add(this.entirebuffer);
            this.writetype.Dock = System.Windows.Forms.DockStyle.Top;
            this.writetype.Location = new System.Drawing.Point(0, 0);
            this.writetype.Name = "writetype";
            this.writetype.Size = new System.Drawing.Size(239, 73);
            this.writetype.TabIndex = 2;
            this.writetype.TabStop = false;
            this.writetype.Text = "Raw Binary Dump";
            // 
            // btn
            // 
            this.btn.Location = new System.Drawing.Point(129, 11);
            this.btn.Name = "btn";
            this.btn.Size = new System.Drawing.Size(102, 28);
            this.btn.TabIndex = 5;
            this.btn.Text = "Write File";
            this.btn.UseVisualStyleBackColor = true;
            this.btn.Click += new System.EventHandler(this.btn_Click);
            // 
            // tagnumber
            // 
            this.tagnumber.Location = new System.Drawing.Point(120, 45);
            this.tagnumber.Maximum = new decimal(new int[] {
            999999999,
            0,
            0,
            0});
            this.tagnumber.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.tagnumber.Name = "tagnumber";
            this.tagnumber.Size = new System.Drawing.Size(78, 20);
            this.tagnumber.TabIndex = 4;
            this.tagnumber.Value = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(204, 50);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(27, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "tags";
            // 
            // mostrecent
            // 
            this.mostrecent.AutoSize = true;
            this.mostrecent.Checked = true;
            this.mostrecent.Location = new System.Drawing.Point(12, 48);
            this.mostrecent.Name = "mostrecent";
            this.mostrecent.Size = new System.Drawing.Size(108, 17);
            this.mostrecent.TabIndex = 1;
            this.mostrecent.TabStop = true;
            this.mostrecent.Text = "Write most recent";
            this.mostrecent.UseVisualStyleBackColor = true;
            this.mostrecent.CheckedChanged += new System.EventHandler(this.mostrecent_CheckedChanged);
            // 
            // entirebuffer
            // 
            this.entirebuffer.AutoSize = true;
            this.entirebuffer.Location = new System.Drawing.Point(12, 27);
            this.entirebuffer.Name = "entirebuffer";
            this.entirebuffer.Size = new System.Drawing.Size(111, 17);
            this.entirebuffer.TabIndex = 0;
            this.entirebuffer.Text = "Write Entire Buffer";
            this.entirebuffer.UseVisualStyleBackColor = true;
            // 
            // filechooser
            // 
            this.filechooser.DefaultExt = "csv";
            // 
            // filewriter
            // 
            this.filewriter.WorkerReportsProgress = true;
            this.filewriter.WorkerSupportsCancellation = true;
            this.filewriter.DoWork += new System.ComponentModel.DoWorkEventHandler(this.filewriter_DoWork);
            this.filewriter.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.filewriter_ProgressChanged);
            this.filewriter.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.filewriter_RunWorkerCompleted);
            // 
            // savedata
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(239, 115);
            this.Controls.Add(this.writetype);
            this.Controls.Add(this.stat);
            this.Controls.Add(this.progress);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "savedata";
            this.Text = "Write Data to File";
            this.writetype.ResumeLayout(false);
            this.writetype.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tagnumber)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar progress;
        private System.Windows.Forms.Label stat;
        private System.Windows.Forms.GroupBox writetype;
        private System.Windows.Forms.Button btn;
        private System.Windows.Forms.NumericUpDown tagnumber;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.RadioButton mostrecent;
        private System.Windows.Forms.RadioButton entirebuffer;
        private System.Windows.Forms.SaveFileDialog filechooser;
        private System.ComponentModel.BackgroundWorker filewriter;
    }
}