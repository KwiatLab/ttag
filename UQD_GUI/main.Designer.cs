namespace TimeTagger
{
    partial class main
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(main));
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.stat = new System.Windows.Forms.RichTextBox();
            this.addrunner = new System.Windows.Forms.Button();
            this.remrunner = new System.Windows.Forms.Button();
            this.refon = new System.Windows.Forms.CheckBox();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.runners = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
            this.ledcontrol = new System.Windows.Forms.ToolStripDropDownButton();
            this.lEDsOFFToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lEDs50ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lEDsMAXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.savemenu = new System.Windows.Forms.ToolStripDropDownButton();
            this.saveToCSVToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveRawTagsToCSVToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.dumpToBinaryFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.optionsmenu = new System.Windows.Forms.ToolStripDropDownButton();
            this.flashbtn = new System.Windows.Forms.ToolStripMenuItem();
            this.edgeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.risingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fallingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.thresholdToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.t2v = new System.Windows.Forms.ToolStripMenuItem();
            this.t15v = new System.Windows.Forms.ToolStripMenuItem();
            this.t1v = new System.Windows.Forms.ToolStripMenuItem();
            this.t05v = new System.Windows.Forms.ToolStripMenuItem();
            this.tn05v = new System.Windows.Forms.ToolStripMenuItem();
            this.tn1v = new System.Windows.Forms.ToolStripMenuItem();
            this.tn15v = new System.Windows.Forms.ToolStripMenuItem();
            this.tn2v = new System.Windows.Forms.ToolStripMenuItem();
            this.threshold_textbox = new System.Windows.Forms.ToolStripTextBox();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.perChannelSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.perchannel_cnumberT = new System.Windows.Forms.ToolStripMenuItem();
            this.perchannel_cnumber = new System.Windows.Forms.ToolStripTextBox();
            this.perchannel_thresholdT = new System.Windows.Forms.ToolStripMenuItem();
            this.perchannel_threshold = new System.Windows.Forms.ToolStripTextBox();
            this.edgeToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.perchannel_edge = new System.Windows.Forms.ToolStripTextBox();
            this.sETCHANNELToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.validateBufferToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tagger = new System.ComponentModel.BackgroundWorker();
            this.runnertimer = new System.Windows.Forms.Timer(this.components);
            this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
            this.tableLayoutPanel1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.toolStripContainer1.BottomToolStripPanel.SuspendLayout();
            this.toolStripContainer1.ContentPanel.SuspendLayout();
            this.toolStripContainer1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 3;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.Controls.Add(this.stat, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.addrunner, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.remrunner, 2, 0);
            this.tableLayoutPanel1.Controls.Add(this.refon, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(284, 240);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // stat
            // 
            this.tableLayoutPanel1.SetColumnSpan(this.stat, 3);
            this.stat.Dock = System.Windows.Forms.DockStyle.Fill;
            this.stat.Location = new System.Drawing.Point(3, 33);
            this.stat.Name = "stat";
            this.stat.ReadOnly = true;
            this.stat.Size = new System.Drawing.Size(278, 204);
            this.stat.TabIndex = 0;
            this.stat.Text = "";
            // 
            // addrunner
            // 
            this.addrunner.Dock = System.Windows.Forms.DockStyle.Fill;
            this.addrunner.Location = new System.Drawing.Point(145, 3);
            this.addrunner.Name = "addrunner";
            this.addrunner.Size = new System.Drawing.Size(65, 24);
            this.addrunner.TabIndex = 2;
            this.addrunner.Text = "Runner +";
            this.addrunner.UseVisualStyleBackColor = true;
            this.addrunner.Click += new System.EventHandler(this.addrunner_Click);
            // 
            // remrunner
            // 
            this.remrunner.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remrunner.Location = new System.Drawing.Point(216, 3);
            this.remrunner.Name = "remrunner";
            this.remrunner.Size = new System.Drawing.Size(65, 24);
            this.remrunner.TabIndex = 3;
            this.remrunner.Text = "Runner -";
            this.remrunner.UseVisualStyleBackColor = true;
            this.remrunner.Click += new System.EventHandler(this.remrunner_Click);
            // 
            // refon
            // 
            this.refon.AutoSize = true;
            this.refon.Dock = System.Windows.Forms.DockStyle.Fill;
            this.refon.Enabled = false;
            this.refon.Location = new System.Drawing.Point(3, 3);
            this.refon.Name = "refon";
            this.refon.Size = new System.Drawing.Size(136, 24);
            this.refon.TabIndex = 4;
            this.refon.Text = "Use 10MHz Reference";
            this.refon.UseVisualStyleBackColor = true;
            this.refon.CheckedChanged += new System.EventHandler(this.refon_CheckedChanged);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Dock = System.Windows.Forms.DockStyle.None;
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.runners,
            this.toolStripStatusLabel2,
            this.ledcontrol,
            this.savemenu,
            this.optionsmenu});
            this.statusStrip1.Location = new System.Drawing.Point(0, 0);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(284, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(53, 17);
            this.toolStripStatusLabel1.Text = "Runners:";
            // 
            // runners
            // 
            this.runners.Name = "runners";
            this.runners.Size = new System.Drawing.Size(13, 17);
            this.runners.Text = "0";
            // 
            // toolStripStatusLabel2
            // 
            this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
            this.toolStripStatusLabel2.Size = new System.Drawing.Size(116, 17);
            this.toolStripStatusLabel2.Spring = true;
            // 
            // ledcontrol
            // 
            this.ledcontrol.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.ledcontrol.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lEDsOFFToolStripMenuItem,
            this.lEDs50ToolStripMenuItem,
            this.lEDsMAXToolStripMenuItem});
            this.ledcontrol.Enabled = false;
            this.ledcontrol.Image = ((System.Drawing.Image)(resources.GetObject("ledcontrol.Image")));
            this.ledcontrol.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.ledcontrol.Name = "ledcontrol";
            this.ledcontrol.Size = new System.Drawing.Size(29, 20);
            this.ledcontrol.Text = "LED Control";
            // 
            // lEDsOFFToolStripMenuItem
            // 
            this.lEDsOFFToolStripMenuItem.Name = "lEDsOFFToolStripMenuItem";
            this.lEDsOFFToolStripMenuItem.Size = new System.Drawing.Size(130, 22);
            this.lEDsOFFToolStripMenuItem.Text = "LEDs OFF";
            this.lEDsOFFToolStripMenuItem.Click += new System.EventHandler(this.lEDsOFFToolStripMenuItem_Click);
            // 
            // lEDs50ToolStripMenuItem
            // 
            this.lEDs50ToolStripMenuItem.Name = "lEDs50ToolStripMenuItem";
            this.lEDs50ToolStripMenuItem.Size = new System.Drawing.Size(130, 22);
            this.lEDs50ToolStripMenuItem.Text = "LEDs 50%";
            this.lEDs50ToolStripMenuItem.Click += new System.EventHandler(this.lEDs50ToolStripMenuItem_Click);
            // 
            // lEDsMAXToolStripMenuItem
            // 
            this.lEDsMAXToolStripMenuItem.Name = "lEDsMAXToolStripMenuItem";
            this.lEDsMAXToolStripMenuItem.Size = new System.Drawing.Size(130, 22);
            this.lEDsMAXToolStripMenuItem.Text = "LEDs 100%";
            this.lEDsMAXToolStripMenuItem.Click += new System.EventHandler(this.lEDsMAXToolStripMenuItem_Click);
            // 
            // savemenu
            // 
            this.savemenu.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.savemenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveToCSVToolStripMenuItem,
            this.saveRawTagsToCSVToolStripMenuItem,
            this.dumpToBinaryFileToolStripMenuItem});
            this.savemenu.Enabled = false;
            this.savemenu.Image = ((System.Drawing.Image)(resources.GetObject("savemenu.Image")));
            this.savemenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.savemenu.Name = "savemenu";
            this.savemenu.Size = new System.Drawing.Size(29, 20);
            this.savemenu.Text = "toolStripDropDownButton1";
            // 
            // saveToCSVToolStripMenuItem
            // 
            this.saveToCSVToolStripMenuItem.Name = "saveToCSVToolStripMenuItem";
            this.saveToCSVToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.saveToCSVToolStripMenuItem.Text = "Save data to CSV";
            this.saveToCSVToolStripMenuItem.Click += new System.EventHandler(this.saveToCSVToolStripMenuItem_Click);
            // 
            // saveRawTagsToCSVToolStripMenuItem
            // 
            this.saveRawTagsToCSVToolStripMenuItem.Name = "saveRawTagsToCSVToolStripMenuItem";
            this.saveRawTagsToCSVToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.saveRawTagsToCSVToolStripMenuItem.Text = "Save raw tags to CSV";
            this.saveRawTagsToCSVToolStripMenuItem.Click += new System.EventHandler(this.saveRawTagsToCSVToolStripMenuItem_Click);
            // 
            // dumpToBinaryFileToolStripMenuItem
            // 
            this.dumpToBinaryFileToolStripMenuItem.Name = "dumpToBinaryFileToolStripMenuItem";
            this.dumpToBinaryFileToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.dumpToBinaryFileToolStripMenuItem.Text = "Dump to binary file";
            this.dumpToBinaryFileToolStripMenuItem.ToolTipText = "File Format: <uint8 channel><uint64 timetag> for each data point";
            this.dumpToBinaryFileToolStripMenuItem.Visible = false;
            this.dumpToBinaryFileToolStripMenuItem.Click += new System.EventHandler(this.dumpToBinaryFileToolStripMenuItem_Click);
            // 
            // optionsmenu
            // 
            this.optionsmenu.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.optionsmenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.flashbtn,
            this.edgeToolStripMenuItem,
            this.thresholdToolStripMenuItem,
            this.perChannelSettingsToolStripMenuItem,
            this.validateBufferToolStripMenuItem});
            this.optionsmenu.Enabled = false;
            this.optionsmenu.Image = ((System.Drawing.Image)(resources.GetObject("optionsmenu.Image")));
            this.optionsmenu.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.optionsmenu.Name = "optionsmenu";
            this.optionsmenu.Size = new System.Drawing.Size(29, 20);
            this.optionsmenu.Text = "toolStripDropDownButton2";
            // 
            // flashbtn
            // 
            this.flashbtn.Name = "flashbtn";
            this.flashbtn.Size = new System.Drawing.Size(183, 22);
            this.flashbtn.Text = "Calibrate";
            this.flashbtn.Click += new System.EventHandler(this.flashbtn_Click);
            // 
            // edgeToolStripMenuItem
            // 
            this.edgeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.risingToolStripMenuItem,
            this.fallingToolStripMenuItem});
            this.edgeToolStripMenuItem.Name = "edgeToolStripMenuItem";
            this.edgeToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.edgeToolStripMenuItem.Text = "Edge";
            // 
            // risingToolStripMenuItem
            // 
            this.risingToolStripMenuItem.Name = "risingToolStripMenuItem";
            this.risingToolStripMenuItem.Size = new System.Drawing.Size(125, 22);
            this.risingToolStripMenuItem.Text = "Rising (+)";
            this.risingToolStripMenuItem.Click += new System.EventHandler(this.risingToolStripMenuItem_Click);
            // 
            // fallingToolStripMenuItem
            // 
            this.fallingToolStripMenuItem.Name = "fallingToolStripMenuItem";
            this.fallingToolStripMenuItem.Size = new System.Drawing.Size(125, 22);
            this.fallingToolStripMenuItem.Text = "Falling (-)";
            this.fallingToolStripMenuItem.Click += new System.EventHandler(this.fallingToolStripMenuItem_Click);
            // 
            // thresholdToolStripMenuItem
            // 
            this.thresholdToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.t2v,
            this.t15v,
            this.t1v,
            this.t05v,
            this.tn05v,
            this.tn1v,
            this.tn15v,
            this.tn2v,
            this.threshold_textbox,
            this.toolStripMenuItem1});
            this.thresholdToolStripMenuItem.Name = "thresholdToolStripMenuItem";
            this.thresholdToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.thresholdToolStripMenuItem.Text = "Threshold";
            // 
            // t2v
            // 
            this.t2v.Enabled = false;
            this.t2v.Name = "t2v";
            this.t2v.Size = new System.Drawing.Size(160, 22);
            this.t2v.Text = "2V";
            this.t2v.Click += new System.EventHandler(this.t2v_Click);
            // 
            // t15v
            // 
            this.t15v.Name = "t15v";
            this.t15v.Size = new System.Drawing.Size(160, 22);
            this.t15v.Text = "1.5V";
            this.t15v.Click += new System.EventHandler(this.t15v_Click);
            // 
            // t1v
            // 
            this.t1v.Name = "t1v";
            this.t1v.Size = new System.Drawing.Size(160, 22);
            this.t1v.Text = "1V";
            this.t1v.Click += new System.EventHandler(this.t1v_Click);
            // 
            // t05v
            // 
            this.t05v.Name = "t05v";
            this.t05v.Size = new System.Drawing.Size(160, 22);
            this.t05v.Text = "0.5V";
            this.t05v.Click += new System.EventHandler(this.t05v_Click);
            // 
            // tn05v
            // 
            this.tn05v.Name = "tn05v";
            this.tn05v.Size = new System.Drawing.Size(160, 22);
            this.tn05v.Text = "-0.5V";
            this.tn05v.Click += new System.EventHandler(this.tn05v_Click);
            // 
            // tn1v
            // 
            this.tn1v.Name = "tn1v";
            this.tn1v.Size = new System.Drawing.Size(160, 22);
            this.tn1v.Text = "-1V";
            this.tn1v.Click += new System.EventHandler(this.tn1v_Click);
            // 
            // tn15v
            // 
            this.tn15v.Name = "tn15v";
            this.tn15v.Size = new System.Drawing.Size(160, 22);
            this.tn15v.Text = "-1.5V";
            this.tn15v.Click += new System.EventHandler(this.tn15v_Click);
            // 
            // tn2v
            // 
            this.tn2v.Name = "tn2v";
            this.tn2v.Size = new System.Drawing.Size(160, 22);
            this.tn2v.Text = "-2V";
            this.tn2v.Click += new System.EventHandler(this.tn2v_Click);
            // 
            // threshold_textbox
            // 
            this.threshold_textbox.Name = "threshold_textbox";
            this.threshold_textbox.Size = new System.Drawing.Size(100, 23);
            this.threshold_textbox.Text = "0.25";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(160, 22);
            this.toolStripMenuItem1.Text = "Set from textbox";
            this.toolStripMenuItem1.Click += new System.EventHandler(this.toolStripMenuItem1_Click);
            // 
            // perChannelSettingsToolStripMenuItem
            // 
            this.perChannelSettingsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.perchannel_cnumberT,
            this.perchannel_cnumber,
            this.perchannel_thresholdT,
            this.perchannel_threshold,
            this.edgeToolStripMenuItem1,
            this.perchannel_edge,
            this.sETCHANNELToolStripMenuItem});
            this.perChannelSettingsToolStripMenuItem.Name = "perChannelSettingsToolStripMenuItem";
            this.perChannelSettingsToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.perChannelSettingsToolStripMenuItem.Text = "Per Channel Settings";
            // 
            // perchannel_cnumberT
            // 
            this.perchannel_cnumberT.Enabled = false;
            this.perchannel_cnumberT.Name = "perchannel_cnumberT";
            this.perchannel_cnumberT.Size = new System.Drawing.Size(186, 22);
            this.perchannel_cnumberT.Text = "Channel Number:";
            // 
            // perchannel_cnumber
            // 
            this.perchannel_cnumber.Name = "perchannel_cnumber";
            this.perchannel_cnumber.Size = new System.Drawing.Size(100, 23);
            this.perchannel_cnumber.Text = "1";
            // 
            // perchannel_thresholdT
            // 
            this.perchannel_thresholdT.Enabled = false;
            this.perchannel_thresholdT.Name = "perchannel_thresholdT";
            this.perchannel_thresholdT.Size = new System.Drawing.Size(186, 22);
            this.perchannel_thresholdT.Text = "Threshold V (-2.0,2.0)";
            // 
            // perchannel_threshold
            // 
            this.perchannel_threshold.Name = "perchannel_threshold";
            this.perchannel_threshold.Size = new System.Drawing.Size(100, 23);
            this.perchannel_threshold.Text = "2.0";
            // 
            // edgeToolStripMenuItem1
            // 
            this.edgeToolStripMenuItem1.Enabled = false;
            this.edgeToolStripMenuItem1.Name = "edgeToolStripMenuItem1";
            this.edgeToolStripMenuItem1.Size = new System.Drawing.Size(186, 22);
            this.edgeToolStripMenuItem1.Text = "Edge (+/-)";
            // 
            // perchannel_edge
            // 
            this.perchannel_edge.Name = "perchannel_edge";
            this.perchannel_edge.Size = new System.Drawing.Size(100, 23);
            this.perchannel_edge.Text = "+";
            // 
            // sETCHANNELToolStripMenuItem
            // 
            this.sETCHANNELToolStripMenuItem.Name = "sETCHANNELToolStripMenuItem";
            this.sETCHANNELToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
            this.sETCHANNELToolStripMenuItem.Text = "Set Channel";
            this.sETCHANNELToolStripMenuItem.Click += new System.EventHandler(this.sETCHANNELToolStripMenuItem_Click);
            // 
            // validateBufferToolStripMenuItem
            // 
            this.validateBufferToolStripMenuItem.Name = "validateBufferToolStripMenuItem";
            this.validateBufferToolStripMenuItem.Size = new System.Drawing.Size(183, 22);
            this.validateBufferToolStripMenuItem.Text = "Validate Buffer";
            this.validateBufferToolStripMenuItem.Click += new System.EventHandler(this.validateBufferToolStripMenuItem_Click);
            // 
            // tagger
            // 
            this.tagger.WorkerSupportsCancellation = true;
            this.tagger.DoWork += new System.ComponentModel.DoWorkEventHandler(this.tagger_DoWork);
            // 
            // runnertimer
            // 
            this.runnertimer.Interval = 700;
            this.runnertimer.Tick += new System.EventHandler(this.runnertimer_Tick);
            // 
            // toolStripContainer1
            // 
            // 
            // toolStripContainer1.BottomToolStripPanel
            // 
            this.toolStripContainer1.BottomToolStripPanel.Controls.Add(this.statusStrip1);
            // 
            // toolStripContainer1.ContentPanel
            // 
            this.toolStripContainer1.ContentPanel.Controls.Add(this.tableLayoutPanel1);
            this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(284, 240);
            this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStripContainer1.LeftToolStripPanelVisible = false;
            this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
            this.toolStripContainer1.Name = "toolStripContainer1";
            this.toolStripContainer1.RightToolStripPanelVisible = false;
            this.toolStripContainer1.Size = new System.Drawing.Size(284, 262);
            this.toolStripContainer1.TabIndex = 2;
            this.toolStripContainer1.Text = "toolStripContainer1";
            this.toolStripContainer1.TopToolStripPanelVisible = false;
            // 
            // main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.toolStripContainer1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "main";
            this.Text = "UQD Time Tagger Interface";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.main_FormClosing);
            this.Load += new System.EventHandler(this.main_Load);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.toolStripContainer1.BottomToolStripPanel.ResumeLayout(false);
            this.toolStripContainer1.BottomToolStripPanel.PerformLayout();
            this.toolStripContainer1.ContentPanel.ResumeLayout(false);
            this.toolStripContainer1.ResumeLayout(false);
            this.toolStripContainer1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.RichTextBox stat;
        private System.Windows.Forms.Button addrunner;
        private System.Windows.Forms.Button remrunner;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel runners;
        private System.ComponentModel.BackgroundWorker tagger;
        private System.Windows.Forms.Timer runnertimer;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
        private System.Windows.Forms.ToolStripDropDownButton ledcontrol;
        private System.Windows.Forms.ToolStripMenuItem lEDsOFFToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem lEDs50ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem lEDsMAXToolStripMenuItem;
        private System.Windows.Forms.CheckBox refon;
        private System.Windows.Forms.ToolStripDropDownButton optionsmenu;
        private System.Windows.Forms.ToolStripMenuItem flashbtn;
        private System.Windows.Forms.ToolStripMenuItem thresholdToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem t2v;
        private System.Windows.Forms.ToolStripMenuItem t15v;
        private System.Windows.Forms.ToolStripMenuItem t1v;
        private System.Windows.Forms.ToolStripMenuItem t05v;
        private System.Windows.Forms.ToolStripMenuItem tn05v;
        private System.Windows.Forms.ToolStripMenuItem tn1v;
        private System.Windows.Forms.ToolStripMenuItem tn15v;
        private System.Windows.Forms.ToolStripMenuItem tn2v;
        private System.Windows.Forms.ToolStripDropDownButton savemenu;
        private System.Windows.Forms.ToolStripMenuItem saveToCSVToolStripMenuItem;
        private System.Windows.Forms.ToolStripContainer toolStripContainer1;
        private System.Windows.Forms.ToolStripMenuItem saveRawTagsToCSVToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem dumpToBinaryFileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem validateBufferToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem edgeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem risingToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fallingToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem perChannelSettingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem perchannel_cnumberT;
        private System.Windows.Forms.ToolStripTextBox perchannel_cnumber;
        private System.Windows.Forms.ToolStripMenuItem perchannel_thresholdT;
        private System.Windows.Forms.ToolStripTextBox perchannel_threshold;
        private System.Windows.Forms.ToolStripMenuItem edgeToolStripMenuItem1;
        private System.Windows.Forms.ToolStripTextBox perchannel_edge;
        private System.Windows.Forms.ToolStripMenuItem sETCHANNELToolStripMenuItem;
        private System.Windows.Forms.ToolStripTextBox threshold_textbox;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
    }
}