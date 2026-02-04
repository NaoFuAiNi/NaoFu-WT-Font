namespace NaoFu.WT.Font.Launcher;

partial class MainForm
{
    private System.ComponentModel.IContainer components = null;
    private System.Windows.Forms.Panel titleBarPanel;
    private System.Windows.Forms.Panel titleBarBorder;
    private System.Windows.Forms.Panel closeBtnPanel;
    private System.Windows.Forms.Panel dropPanel;
    private Microsoft.Web.WebView2.WinForms.WebView2 webView;

    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null))
            components.Dispose();
        base.Dispose(disposing);
    }

    private void InitializeComponent()
    {
        this.titleBarPanel = new System.Windows.Forms.Panel();
        this.titleBarBorder = new System.Windows.Forms.Panel();
        this.closeBtnPanel = new System.Windows.Forms.Panel();
        this.dropPanel = new System.Windows.Forms.Panel();
        this.webView = new Microsoft.Web.WebView2.WinForms.WebView2();
        this.titleBarPanel.SuspendLayout();
        this.dropPanel.SuspendLayout();
        ((System.ComponentModel.ISupportInitialize)(this.webView)).BeginInit();
        this.SuspendLayout();
        //
        // titleBarPanel
        //
        this.titleBarPanel.BackColor = System.Drawing.Color.Transparent;
        this.titleBarPanel.Controls.Add(this.titleBarBorder);
        this.titleBarPanel.Controls.Add(this.closeBtnPanel);
        this.titleBarPanel.Dock = System.Windows.Forms.DockStyle.Top;
        this.titleBarPanel.Location = new System.Drawing.Point(0, 0);
        this.titleBarPanel.Name = "titleBarPanel";
        this.titleBarPanel.Size = new System.Drawing.Size(1600, 46);
        this.titleBarPanel.TabIndex = 1;
        this.titleBarPanel.Paint += TitleBarPanel_Paint;
        this.titleBarPanel.MouseDown += TitleBarPanel_MouseDown;
        //
        // titleBarBorder
        //
        this.titleBarBorder.BackColor = System.Drawing.Color.Transparent;
        this.titleBarBorder.Dock = System.Windows.Forms.DockStyle.Bottom;
        this.titleBarBorder.Location = new System.Drawing.Point(0, 45);
        this.titleBarBorder.Name = "titleBarBorder";
        this.titleBarBorder.Size = new System.Drawing.Size(1600, 1);
        this.titleBarBorder.TabIndex = 2;
        //
        // closeBtnPanel
        //
        this.closeBtnPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
        this.closeBtnPanel.BackColor = System.Drawing.Color.Transparent;
        this.closeBtnPanel.Cursor = System.Windows.Forms.Cursors.Hand;
        this.closeBtnPanel.Location = new System.Drawing.Point(1552, 0);
        this.closeBtnPanel.Name = "closeBtnPanel";
        this.closeBtnPanel.Size = new System.Drawing.Size(48, 46);
        this.closeBtnPanel.TabIndex = 1;
        this.closeBtnPanel.Paint += CloseBtnPanel_Paint;
        this.closeBtnPanel.Click += CloseBtnPanel_Click;
        this.closeBtnPanel.MouseEnter += CloseBtnPanel_MouseEnter;
        this.closeBtnPanel.MouseLeave += CloseBtnPanel_MouseLeave;
        //
        // dropPanel
        //
        this.dropPanel.AllowDrop = true;
        this.dropPanel.Controls.Add(this.webView);
        this.dropPanel.Dock = System.Windows.Forms.DockStyle.Fill;
        this.dropPanel.Location = new System.Drawing.Point(0, 46);
        this.dropPanel.Name = "dropPanel";
        this.dropPanel.Size = new System.Drawing.Size(1600, 904);
        this.dropPanel.TabIndex = 0;
        //
        // webView
        //
        this.webView.AllowExternalDrop = false;
        this.webView.CreationProperties = null;
        this.webView.DefaultBackgroundColor = System.Drawing.Color.White;
        this.webView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.webView.Location = new System.Drawing.Point(0, 0);
        this.webView.Name = "webView";
        this.webView.Size = new System.Drawing.Size(1600, 904);
        this.webView.TabIndex = 0;
        this.webView.ZoomFactor = 1D;
        //
        // MainForm
        //
        this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 17F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
        this.BackColor = System.Drawing.Color.FromArgb(232, 244, 252);
        this.ClientSize = new System.Drawing.Size(1600, 950);
        this.Controls.Add(this.dropPanel);
        this.Controls.Add(this.titleBarPanel);
        this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
        this.MinimumSize = new System.Drawing.Size(720, 480);
        this.Name = "MainForm";
        this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
        this.Text = "NaoFu";
        this.titleBarPanel.ResumeLayout(false);
        this.dropPanel.ResumeLayout(false);
        ((System.ComponentModel.ISupportInitialize)(this.webView)).EndInit();
        this.ResumeLayout(false);
    }
}
