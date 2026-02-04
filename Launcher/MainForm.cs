using System.Runtime.InteropServices;
using Microsoft.Web.WebView2.WinForms;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using Microsoft.Win32;

namespace NaoFu.WT.Font.Launcher;

public partial class MainForm : Form
{
    private Bridge? _bridge;
    private bool _titleBarDarkTheme;
    private bool _closeBtnHover;
    private readonly PrivateFontCollection _titleFontCollection = new();
    private bool _savedThemeDark;
    private string _titleBarFontFamily = "SimHei";
    private Image? _titleBarLogo;
    private const int TitleBarLogoSize = 60;
    private const int TitleBarLogoLeft = -2;
    private const int TitleBarTextLeft = 63;
    private bool _initialShowDone;

    [DllImport("user32.dll")]
    private static extern int ReleaseCapture();

    [DllImport("user32.dll")]
    private static extern int SendMessage(IntPtr hWnd, int msg, int wParam, int lParam);

    [DllImport("dwmapi.dll", PreserveSig = true)]
    private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int value, int size);

    private const int WM_NCLBUTTONDOWN = 0xA1;
    private const int HTCAPTION = 0x2;
    private const int DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    private const int DWM_WINDOW_CORNER_ROUND = 2;

    private static bool IsWindows11OrLater()
    {
        try
        {
            using var key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion");
            var build = key?.GetValue("CurrentBuild") as string;
            if (string.IsNullOrEmpty(build) || !int.TryParse(build, out var buildNum)) return false;
            return buildNum >= 22000;
        }
        catch { return false; }
    }

    public MainForm()
    {
        InitializeComponent();
        Opacity = 0; // 先隐藏，等初始化完成后再显示
        _savedThemeDark = Bridge.GetSavedTheme();
        _titleBarDarkTheme = _savedThemeDark;
        BackColor = _savedThemeDark ? Color.FromArgb(22, 28, 42) : Color.FromArgb(232, 244, 252);
        if (titleBarBorder != null)
            titleBarBorder.BackColor = _savedThemeDark ? Color.FromArgb(18, 24, 38) : Color.FromArgb(224, 232, 240);
        SetDoubleBuffered(titleBarPanel);
        SetDoubleBuffered(closeBtnPanel);
        TryLoadTitleBarFont();
    }

    private void TryLoadTitleBarFont()
    {
        var baseDir = Bridge.GetAppRoot();
        var fontPath = Path.Combine(baseDir, "assets", "CuteFount.ttf");
        if (File.Exists(fontPath))
        {
            try
            {
                _titleFontCollection.AddFontFile(fontPath);
                if (_titleFontCollection.Families.Length > 0)
                    _titleBarFontFamily = _titleFontCollection.Families[0].Name;
            }
            catch { /* 失败则用 SimHei */ }
        }
    }

    private static void SetDoubleBuffered(Control ctrl)
    {
        typeof(Control).InvokeMember("DoubleBuffered",
            System.Reflection.BindingFlags.SetProperty | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic,
            null, ctrl, new object[] { true });
    }

    protected override async void OnLoad(EventArgs e)
    {
        base.OnLoad(e);
        // 仅在 Win11 上启用窗口圆角，Win10 保持直角
        if (IsWindows11OrLater() && IsHandleCreated)
        {
            int preference = DWM_WINDOW_CORNER_ROUND;
            DwmSetWindowAttribute(Handle, DWMWA_WINDOW_CORNER_PREFERENCE, ref preference, 4);
        }
        try
        {
            await webView.EnsureCoreWebView2Async(null);
            // 禁用开发者工具与相关入口，避免 F12/右键检查/快捷键
            var settings = webView.CoreWebView2.Settings;
            settings.AreDevToolsEnabled = false;
            settings.AreDefaultContextMenusEnabled = false;
            settings.AreBrowserAcceleratorKeysEnabled = false;
            settings.IsStatusBarEnabled = false; // 鼠标悬停卡片/链接时不显示左下角 URL
            _bridge = new Bridge(webView, isDark =>
            {
                if (IsDisposed || !IsHandleCreated) return;
                if (InvokeRequired)
                    BeginInvoke(() => SetTitleBarTheme(isDark));
                else
                    SetTitleBarTheme(isDark);
            });
            webView.CoreWebView2.AddHostObjectToScript("nfBridge", _bridge);
            webView.CoreWebView2.NavigationCompleted += (_, _) =>
            {
                if (IsDisposed || !IsHandleCreated) return;
                var url = webView.CoreWebView2?.Source ?? "";
                if (url.Contains("version_dark", StringComparison.OrdinalIgnoreCase))
                    SetTitleBarTheme(true);
                else if (url.Contains("version_light", StringComparison.OrdinalIgnoreCase))
                    SetTitleBarTheme(false);
                if (_initialShowDone) return;
                _initialShowDone = true;
                if (InvokeRequired) BeginInvoke(() => { Opacity = 1; Activate(); });
                else { Opacity = 1; Activate(); }
            };
            SetTitleBarTheme(_savedThemeDark);
            dropPanel.DragEnter += DropPanel_DragEnter;
            dropPanel.DragOver += DropPanel_DragOver;
            dropPanel.DragDrop += DropPanel_Drop;
            var appRoot = Bridge.GetAppRoot();
            var exeDir = Application.StartupPath ?? "";
            string uiPath = Path.Combine(appRoot, "ui", "welcome.html");
            if (!File.Exists(uiPath) && !string.IsNullOrEmpty(exeDir))
                uiPath = Path.Combine(exeDir, "ui", "welcome.html");
            if (File.Exists(uiPath))
            {
                string uri = new Uri(Path.GetFullPath(uiPath)).AbsoluteUri;
                webView.Source = new Uri(uri);
            }
            else
            {
                webView.NavigateToString("<body style='font-family:\"ZCOOL Kuaile\",\"Microsoft YaHei UI\",\"Microsoft YaHei\",sans-serif;-webkit-font-smoothing:antialiased;padding:20px;'><h2>未找到 ui/welcome.html</h2><p>请将 Launcher 放在项目根目录（与 ui 文件夹同级）后运行。</p></body>");
            }
        }
        catch (Exception ex)
        {
            webView.NavigateToString($"<body style='font-family:\"ZCOOL Kuaile\",\"Microsoft YaHei UI\",\"Microsoft YaHei\",sans-serif;-webkit-font-smoothing:antialiased;padding:20px;'><h2>WebView2 加载失败</h2><pre>{System.Net.WebUtility.HtmlEncode(ex.Message)}</pre></body>");
        }
        TryLoadTitleBarLogo();
    }

    private void TryLoadTitleBarLogo()
    {
        var baseDir = Bridge.GetAppRoot();
        var assetsDir = Path.Combine(baseDir, "assets");
        // 支持 app_ico.png（下划线）和 app-icon.png（连字符）
        var logoPath = Path.Combine(assetsDir, "app_ico.png");
        if (!File.Exists(logoPath))
            logoPath = Path.Combine(assetsDir, "app-icon.png");
        if (File.Exists(logoPath))
        {
            try
            {
                _titleBarLogo = Image.FromFile(logoPath);
            }
            catch { /* 失败则无 logo */ }
        }
    }

    protected override void OnFormClosing(FormClosingEventArgs e)
    {
        _titleBarLogo?.Dispose();
        _titleBarLogo = null;
        base.OnFormClosing(e);
    }

    private static bool IsFontFile(string path)
    {
        if (string.IsNullOrEmpty(path)) return false;
        var ext = Path.GetExtension(path).ToLowerInvariant();
        return ext == ".ttf" || ext == ".otf";
    }

    private void DropPanel_DragEnter(object? sender, DragEventArgs e)
    {
        if (e.Data?.GetDataPresent(DataFormats.FileDrop) == true)
        {
            var files = e.Data.GetData(DataFormats.FileDrop) as string[];
            if (files != null && files.Length > 0 && IsFontFile(files[0]))
                e.Effect = DragDropEffects.Copy;
        }
    }

    private void DropPanel_DragOver(object? sender, DragEventArgs e)
    {
        if (e.Data?.GetDataPresent(DataFormats.FileDrop) == true)
        {
            var files = e.Data.GetData(DataFormats.FileDrop) as string[];
            if (files != null && files.Length > 0 && IsFontFile(files[0]))
                e.Effect = DragDropEffects.Copy;
        }
    }

    private void TitleBarPanel_MouseDown(object? sender, MouseEventArgs e)
    {
        if (e.Button == MouseButtons.Left)
        {
            ReleaseCapture();
            SendMessage(Handle, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
    }

    private void CloseBtnPanel_Click(object? sender, EventArgs e) => Close();

    private void CloseBtnPanel_MouseEnter(object? sender, EventArgs e)
    {
        _closeBtnHover = true;
        closeBtnPanel.Invalidate();
    }

    private void CloseBtnPanel_MouseLeave(object? sender, EventArgs e)
    {
        _closeBtnHover = false;
        closeBtnPanel.Invalidate();
    }

    public void SetTitleBarTheme(bool isDark)
    {
        _titleBarDarkTheme = isDark;
        BackColor = isDark ? Color.FromArgb(22, 28, 42) : Color.FromArgb(232, 244, 252);
        if (titleBarBorder != null)
            titleBarBorder.BackColor = isDark ? Color.FromArgb(18, 24, 38) : Color.FromArgb(224, 232, 240);
        titleBarPanel?.Invalidate();
        closeBtnPanel?.Invalidate();
        Bridge.SaveTheme(isDark);
    }

    private void TitleBarPanel_Paint(object? sender, PaintEventArgs e)
    {
        var g = e.Graphics;
        g.SmoothingMode = SmoothingMode.HighQuality;
        g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
        var r = titleBarPanel.ClientRectangle;
        if (r.Width <= 0 || r.Height <= 0) return;

        using (var brush = new LinearGradientBrush(r,
            _titleBarDarkTheme ? Color.FromArgb(22, 28, 42) : Color.FromArgb(232, 244, 252),
            _titleBarDarkTheme ? Color.FromArgb(15, 15, 20) : Color.FromArgb(224, 236, 248),
            LinearGradientMode.Vertical))
        {
            g.FillRectangle(brush, r);
        }

        if (_titleBarLogo != null)
        {
            int logoY = (r.Height - TitleBarLogoSize) / 2;
            g.InterpolationMode = InterpolationMode.HighQualityBicubic;
            int w = _titleBarLogo.Width;
            int h = _titleBarLogo.Height;
            // 四周轻微虚化 + 左右镜像：目标矩形用负宽实现水平翻转
            const int expand = 2;
            const float softAlpha = 0.06f;
            using (var attrs = new ImageAttributes())
            {
                attrs.SetColorMatrix(new ColorMatrix(new float[][]
                {
                    new float[] { 1, 0, 0, 0, 0 },
                    new float[] { 0, 1, 0, 0, 0 },
                    new float[] { 0, 0, 1, 0, 0 },
                    new float[] { 0, 0, 0, softAlpha, 0 },
                    new float[] { 0, 0, 0, 0, 1 }
                }));
                int s = TitleBarLogoSize + expand * 2;
                int x = TitleBarLogoLeft - expand;
                int y = logoY - expand;
                g.DrawImage(_titleBarLogo, new Rectangle(x + s, y, -s, s), 0, 0, w, h, GraphicsUnit.Pixel, attrs);
            }
            g.DrawImage(_titleBarLogo, new Rectangle(TitleBarLogoLeft + TitleBarLogoSize, logoY, -TitleBarLogoSize, TitleBarLogoSize));
        }

        using (var titleFont = new System.Drawing.Font(_titleBarFontFamily, 16f, FontStyle.Bold))
        {
            const string title = "NaoFu";
            var titleSize = g.MeasureString(title, titleFont);
            float titleY = (r.Height - titleSize.Height) / 2f + 4f;
            var titleRect = new RectangleF(TitleBarTextLeft, titleY, r.Width - TitleBarTextLeft - 50, titleSize.Height);
            var c1 = _titleBarDarkTheme ? Color.FromArgb(56, 189, 248) : Color.FromArgb(91, 143, 185);
            var c2 = _titleBarDarkTheme ? Color.FromArgb(167, 139, 250) : Color.FromArgb(184, 122, 163);
            using (var textBrush = new LinearGradientBrush(new Rectangle(TitleBarTextLeft, 0, (int)titleRect.Width, (int)titleRect.Height), c1, c2, 0f))
            {
                g.DrawString(title, titleFont, textBrush, titleRect.X, titleRect.Y);
            }
        }
    }

    private void CloseBtnPanel_Paint(object? sender, PaintEventArgs e)
    {
        var g = e.Graphics;
        g.SmoothingMode = SmoothingMode.HighQuality;
        var r = closeBtnPanel.ClientRectangle;
        if (r.Width <= 0 || r.Height <= 0) return;

        if (_closeBtnHover)
        {
            var hoverBg = _titleBarDarkTheme ? Color.FromArgb(139, 92, 246) : Color.FromArgb(248, 113, 113);
            using (var brush = new SolidBrush(hoverBg))
            {
                // 多画 1 像素高度，盖住标题栏顶部可能出现的白线
                g.FillRectangle(brush, 0, -1, r.Width, r.Height + 1);
            }
        }

        float pad = Math.Min(r.Width, r.Height) * 0.26f;
        if (pad < 6f) pad = 6f;
        float x1 = pad;
        float y1 = pad;
        float x2 = r.Width - pad;
        float y2 = r.Height - pad;
        Color lineColor = _closeBtnHover ? Color.White : (_titleBarDarkTheme ? Color.FromArgb(226, 232, 240) : Color.FromArgb(71, 85, 105));
        float lineW = Math.Max(1.8f, Math.Min(r.Width, r.Height) / 16f);
        using (var pen = new Pen(lineColor, lineW))
        {
            g.DrawLine(pen, x1, y1, x2, y2);
            g.DrawLine(pen, x2, y1, x1, y2);
        }
    }

    private async void DropPanel_Drop(object? sender, DragEventArgs e)
    {
        if (_bridge == null || e.Data?.GetData(DataFormats.FileDrop) is not string[] files || files.Length == 0)
            return;
        var path = files[0];
        if (!IsFontFile(path)) return;
        var json = _bridge.CopyFontFromPath(path);
        try
        {
            var doc = System.Text.Json.JsonDocument.Parse(json);
            if (doc.RootElement.TryGetProperty("ok", out var ok) && ok.GetBoolean() &&
                doc.RootElement.TryGetProperty("sourcePath", out var sp))
            {
                var sourcePath = sp.GetString() ?? "";
                var arg = System.Text.Json.JsonSerializer.Serialize(sourcePath);
                await (webView.CoreWebView2?.ExecuteScriptAsync(
                    "typeof window.onNfFontDropped === 'function' && window.onNfFontDropped(" + arg + ");") ?? Task.CompletedTask);
            }
        }
        catch { }
    }
}
