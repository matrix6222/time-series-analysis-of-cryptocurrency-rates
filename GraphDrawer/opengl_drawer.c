//gcc -shared -o opengl_drawer.dll opengl_drawer.c -lopengl32 -lgdi32

#include <Windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <float.h>
#include <time.h>


HWND hWnd;
HGLRC hglrc;
GLuint fontList;
HFONT hFont_info;
HFONT hFont_time;
HFONT hFont_time_counter;
int font_info_height = 20;
int font_info_width = 9;
int font_time_height = 10;
int font_time_width = 5;
int font_time_counter_height = 60;
int font_time_counter_width = 28;


void Initialize(HWND hwnd) {
	hWnd = hwnd;
	HDC hdc = GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, };
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);
	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	fontList = glGenLists(96);
	hFont_info = CreateFont(font_info_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, TEXT("Consolas"));
	hFont_time = CreateFont(font_time_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, TEXT("Consolas"));
    hFont_time_counter = CreateFont(font_time_counter_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, TEXT("Consolas"));

	glLoadIdentity();
	glClearColor(0.08627451f, 0.101960786f, 0.11764706f, 0.0f);

	ReleaseDC(hWnd, hdc);
}

void renderText(const char* text, int x, int y) {
	glPushMatrix();
	glRasterPos2i(x, y);
	glPushAttrib(GL_LIST_BIT);
	glListBase(fontList - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
	glPopMatrix();
}

void renderGraph(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int mouse_idx,
    int width_known,
    int size_x,
    int data_rows,
    float* data,
    int analyse1_config_rows,
    int* analyse1_config,
    int analyse1_data_cols,
    float* analyse1_data,

    int nn_active,
    float nn_selected_value,
    int nn_selected_label,
    float nn_btc_value,
    int nn_btc_label,
    float nn_eth_value,
    int nn_eth_label,
    float nn_ltc_value,
    int nn_ltc_label,
    int nn_r,
    int nn_g,
    int nn_b
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y -  wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    // draw actual minute rect
    glBegin(GL_QUADS);
    glColor3ub(22 + 5, 26 + 5, 30 + 5);
    glVertex2i(width_known, 0);
    glVertex2i(width_known + size_x, 0);
    glVertex2i(width_known + size_x, wnd_size_y);
    glVertex2i(width_known, wnd_size_y);
    glEnd();

    // draw mouse |
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();
    }

    int data_cols = 6;
    int amount_known = width_known / size_x;
    int target_col_idx[4] = { 0, 1, 2, 3 };
    float maxi_graph_price = FLT_MIN;
    float mini_graph_price = FLT_MAX;

    // calculate mini and maxi for candles
    for (int y = 0; y < amount_known && y < data_rows; y++) {
        int target_row = data_rows - y - 1;
        for (int x = 0; x < 4; x++) {
            int col = target_col_idx[x];
            float value = data[target_row * data_cols + col];
            if (value > maxi_graph_price) {
                maxi_graph_price = value;
            }
            if (value < mini_graph_price) {
                mini_graph_price = value;
            }
        }
    }

    float scale = (wnd_y1 - wnd_y0) / (maxi_graph_price - mini_graph_price);

    // draw candles
    for (int y = 0; y < amount_known && y < data_rows; y++) {
        int target_row = data_rows - y - 1;
        float  open_price = (data[target_row * data_cols + target_col_idx[0]] - mini_graph_price) * scale;
        float close_price = (data[target_row * data_cols + target_col_idx[3]] - mini_graph_price) * scale;
        float  high_price = (data[target_row * data_cols + target_col_idx[1]] - mini_graph_price) * scale;
        float   low_price = (data[target_row * data_cols + target_col_idx[2]] - mini_graph_price) * scale;
        int x2 = width_known - y * size_x;
        int x1 = x2 - size_x;
        int xm = x1 + size_x / 2;
        x2--;

        if (open_price > close_price) {
            glColor3ub(246, 70, 93);
            glBegin(GL_LINES);
            glVertex2i(xm, high_price);
            glVertex2i(xm, low_price);
            glEnd();
            if (abs((int)open_price - (int)close_price) < 1) {
                glBegin(GL_QUADS);
                glVertex2i(x1, open_price);
                glVertex2i(x2, open_price);
                glVertex2i(x2, close_price - 1);
                glVertex2i(x1, close_price - 1);
                glEnd();
            }
            else {
                glBegin(GL_QUADS);
                glVertex2i(x1, open_price);
                glVertex2i(x2, open_price);
                glVertex2i(x2, close_price);
                glVertex2i(x1, close_price);
                glEnd();
            }
        }
        else {
            glColor3ub(14, 203, 129);
            glBegin(GL_LINES);
            glVertex2i(xm, high_price);
            glVertex2i(xm, low_price);
            glEnd();
            if (abs((int)open_price - (int)close_price) < 1) {
                glBegin(GL_QUADS);
                glVertex2i(x1, open_price);
                glVertex2i(x2, open_price);
                glVertex2i(x2, close_price - 1);
                glVertex2i(x1, close_price - 1);
                glEnd();
            }
            else {
                glBegin(GL_QUADS);
                glVertex2i(x1, open_price);
                glVertex2i(x2, open_price);
                glVertex2i(x2, close_price);
                glVertex2i(x1, close_price);
                glEnd();
            }
        }
    }

    // draw analyse SMA, EMA, WMA, VWAP, TRIX
    for (int y = 0; y < analyse1_config_rows; y++) {
        int type = analyse1_config[y * 5 + 0];
        if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5) {
            int len = analyse1_config[y * 5 + 1];
            int color_r = analyse1_config[y * 5 + 2];
            int color_g = analyse1_config[y * 5 + 3];
            int color_b = analyse1_config[y * 5 + 4];
            for (int x = 0; x < amount_known - 1 && x < len - 1; x++) {
                int target_column_2 = len - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse1_data[y * analyse1_data_cols + target_column_1] - mini_graph_price) * scale;
                float y2 = (analyse1_data[y * analyse1_data_cols + target_column_2] - mini_graph_price) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color_r, color_g, color_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    // draw neural network prediction
    if (nn_active == 1) {
        // config
        int left = width_known;
        int arrow_x_left = left;
        int arrow_x_right = left + 81;
        int arrow_y_up = wnd_size_y * 0.75;
        int arrow_y_down = wnd_size_y * 0.25;

        int arrow_x_middle = (arrow_x_left + arrow_x_right) / 2;
        int arrow_y_middle = (arrow_y_up + arrow_y_down) / 2;
        int arrow_y_size = arrow_y_up - arrow_y_down;

        // draw info
        glColor3ub(94, 101, 112);
        renderText("BTC: ", left, wnd_size_y - 2 * font_info_height);
        renderText("LTC: ", left, wnd_size_y - 3 * font_info_height);
        renderText("ETH: ", left, wnd_size_y - 4 * font_info_height);

        int bufferSize = snprintf(NULL, 0, "%.2f", nn_btc_value);
        char* myString = (char*)malloc(bufferSize + 1);
        snprintf(myString, bufferSize + 1, "%.2f", nn_btc_value);
        if (nn_btc_label == 0) {
            glColor3ub(246, 70, 93);
        }
        else if (nn_btc_label == 1) {
            glColor3ub(94, 101, 112);
        }
        else if (nn_btc_label == 2) {
            glColor3ub(14, 203, 129);
        }
        renderText(myString, left + 5 * font_info_width, wnd_size_y - 2 * font_info_height);
        free(myString);

        bufferSize = snprintf(NULL, 0, "%.2f", nn_ltc_value);
        myString = (char*)malloc(bufferSize + 1);
        snprintf(myString, bufferSize + 1, "%.2f", nn_ltc_value);
        if (nn_ltc_label == 0) {
            glColor3ub(246, 70, 93);
        }
        else if (nn_ltc_label == 1) {
            glColor3ub(94, 101, 112);
        }
        else if (nn_ltc_label == 2) {
            glColor3ub(14, 203, 129);
        }
        renderText(myString, left + 5 * font_info_width, wnd_size_y - 3 * font_info_height);
        free(myString);

        bufferSize = snprintf(NULL, 0, "%.2f", nn_eth_value);
        myString = (char*)malloc(bufferSize + 1);
        snprintf(myString, bufferSize + 1, "%.2f", nn_eth_value);
        if (nn_eth_label == 0) {
            glColor3ub(246, 70, 93);
        }
        else if (nn_eth_label == 1) {
            glColor3ub(94, 101, 112);
        }
        else if (nn_eth_label == 2) {
            glColor3ub(14, 203, 129);
        }
        renderText(myString, left + 5 * font_info_width, wnd_size_y - 4 * font_info_height);
        free(myString);

        // draw arrow
        int middle_y = wnd_size_y / 2;

        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(arrow_x_left, arrow_y_middle);
        glVertex2i(arrow_x_right, arrow_y_middle);
        glEnd();

        if (nn_selected_label == 0) {
            glColor3ub(nn_r, nn_g, nn_b);
        }
        else if (nn_selected_label == 1) {
            glColor3ub(94, 101, 112);
        }
        else if (nn_selected_label == 2) {
            glColor3ub(nn_r, nn_g, nn_b);
        }
        int arrow_y_end = arrow_y_down + nn_selected_value * arrow_y_size;
        glBegin(GL_LINES);
        glVertex2i(arrow_x_middle, arrow_y_middle);
        glVertex2i(arrow_x_middle, arrow_y_end);
        glEnd();
    }

    // draw mouse - and prices
    if (mouse_in_canvas == 1) {
        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = (maxi_graph_price - mini_graph_price) * ((float)mouse_y / (float)wnd_size_y) + mini_graph_price;
            int bufferSize = snprintf(NULL, 0, "%.2f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%.2f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void renderTimeline(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int width_known,
    int size_x,
    UINT64 timestamp
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    glBegin(GL_QUADS);
    glColor3ub(22 - 10, 26 - 10, 30 - 10);
    glVertex2i(wnd_x0, wnd_y0);
    glVertex2i(wnd_x1, wnd_y0);
    glVertex2i(wnd_x1, wnd_y1);
    glVertex2i(wnd_x0, wnd_y1);
    glEnd();

    int amount_known = width_known / size_x;
    int start_px = width_known + 2 * size_x;
    time_t seconds = timestamp / 1000;
    seconds = seconds + 3 * 60;
    struct tm* timeinfo = localtime(&seconds);
    char myString[6];
    glColor3ub(94, 101, 112);
    if (size_x >= 40) {
        for (int y = 0; y < amount_known + 3; y++) {
            snprintf(myString, 6, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
            renderText(myString, start_px - y * size_x - 2.5f * font_time_width, wnd_size_y - 8);
            seconds = seconds - 60;
            struct tm* timeinfo = localtime(&seconds);
        }
    }
    else if (size_x >= 20 && size_x < 40) {
        for (int y = 0; y < amount_known + 3; y = y + 2) {
            snprintf(myString, 6, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
            renderText(myString, start_px - y * size_x - 2.5f * font_time_width, wnd_size_y - 8);
            seconds = seconds - 120;
            struct tm* timeinfo = localtime(&seconds);
        }
    }
    else if (size_x >= 15 && size_x < 20) {
        for (int y = 0; y < amount_known + 3; y = y + 3) {
            snprintf(myString, 6, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
            renderText(myString, start_px - y * size_x - 2.5f * font_time_width, wnd_size_y - 8);
            seconds = seconds - 180;
            struct tm* timeinfo = localtime(&seconds);
        }
    }
    else if (size_x >= 10 && size_x < 15) {
        for (int y = 0; y < amount_known + 3; y = y + 4) {
            snprintf(myString, 6, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
            renderText(myString, start_px - y * size_x - 2.5f * font_time_width, wnd_size_y - 8);
            seconds = seconds - 240;
            struct tm* timeinfo = localtime(&seconds);
        }
    }
    else if (size_x < 10) {
        for (int y = 0; y < amount_known + 3; y = y + 7) {
            snprintf(myString, 6, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
            renderText(myString, start_px - y * size_x - 2.5f * font_time_width, wnd_size_y - 8);
            seconds = seconds - 420;
            struct tm* timeinfo = localtime(&seconds);
        }
    }
}

void renderVolumes(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int width_known,
    int size_x,
    int data_rows,
    float* data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    // draw actual minute rect
    glBegin(GL_QUADS);
    glColor3ub(22 + 5, 26 + 5, 30 + 5);
    glVertex2i(width_known, 0);
    glVertex2i(width_known + size_x, 0);
    glVertex2i(width_known + size_x, wnd_size_y);
    glVertex2i(width_known, wnd_size_y);
    glEnd();

    // draw mouse |
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();
    }

    int volumes_middle = wnd_size_y / 2;
    int data_cols = 6;
    int amount_known = width_known / size_x;
    int target_cols_volumes[2] = { 4, 5 };
    float maxi_volume = FLT_MIN;
    float mini_volume = 0.0f;

    // calculate mini and maxi for volumes
    for (int y = 0; y < amount_known && y < data_rows; y++) {
        int target_row = data_rows - y - 1;
        float value = data[target_row * data_cols + target_cols_volumes[0]];
        if (value > maxi_volume) {
            maxi_volume = value;
        }
    }

    float scale = ((float)wnd_size_y / 2) / maxi_volume;

    // draw volumes
    for (int y = 0; y < amount_known && y < data_rows; y++) {
        int target_row = data_rows - y - 1;
        float volume_all = data[target_row * data_cols + target_cols_volumes[0]];
        float volume_buy = data[target_row * data_cols + target_cols_volumes[1]];
        float volume_sell = volume_all - volume_buy;
        volume_all = volume_all * scale;
        volume_buy = volume_buy * scale;
        volume_sell = volume_sell * scale;

        int x2 = width_known - y * size_x;
        int x1 = x2 - size_x;
        x2--;

        glBegin(GL_QUADS);

        glColor3ub(68, 68, 68);
        glVertex2i(x1, volumes_middle);
        glVertex2i(x2, volumes_middle);
        glVertex2i(x2, volumes_middle + volume_all);
        glVertex2i(x1, volumes_middle + volume_all);

        glColor3ub(246, 70, 93);
        glVertex2i(x1, volumes_middle);
        glVertex2i(x2, volumes_middle);
        glVertex2i(x2, volumes_middle + volume_buy);
        glVertex2i(x1, volumes_middle + volume_buy);

        glColor3ub(14, 203, 129);
        glVertex2i(x1, volumes_middle - volume_sell);
        glVertex2i(x2, volumes_middle - volume_sell);
        glVertex2i(x2, volumes_middle);
        glVertex2i(x1, volumes_middle);

        glEnd();
    }

    // draw mouse - and prices
    if (mouse_in_canvas == 1) {
        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = 0.0f;
            if (mouse_y < volumes_middle) {
                float scale = maxi_volume / volumes_middle;
                value = (volumes_middle - mouse_y) * scale;
            }
            else {
                float scale = ((float)mouse_y - volumes_middle) / volumes_middle;
                value = (maxi_volume - mini_volume) * scale;
            }

            int bufferSize = snprintf(NULL, 0, "%f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void renderInfo(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_idx,
    int width_known,
    int size_x,
    int data_rows,
    float* data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    int target_col_idx[6] = { 0, 1, 2, 3, 4, 5 };
    int data_cols = 6;

    glColor3ub(94, 101, 112);
    renderText(" Open:", 0 * font_info_width, wnd_size_y - 1 * font_info_height);
    renderText("Close:", 0 * font_info_width, wnd_size_y - 2 * font_info_height);
    renderText("High:", 20 * font_info_width, wnd_size_y - 1 * font_info_height);
    renderText(" Low:", 20 * font_info_width, wnd_size_y - 2 * font_info_height);
    renderText(" Volume-Buy:", 39 * font_info_width, wnd_size_y - 1 * font_info_height);
    renderText("Volume-Sell:", 39 * font_info_width, wnd_size_y - 2 * font_info_height);

    if (mouse_in_canvas == 1) {
        int amount_known = width_known / size_x;
        if (mouse_idx < amount_known && data_rows > 0) {
            mouse_idx = mouse_idx - amount_known;
            if (abs(mouse_idx) <= data_rows) {
                int target_row = data_rows + mouse_idx;

                float open_price = data[target_row * data_cols + target_col_idx[0]];
                float close_price = data[target_row * data_cols + target_col_idx[3]];
                float high_price = data[target_row * data_cols + target_col_idx[1]];
                float low_price = data[target_row * data_cols + target_col_idx[2]];
                float volume_all = data[target_row * data_cols + target_col_idx[4]];
                float volume_buy = data[target_row * data_cols + target_col_idx[5]];
                float volume_sell = volume_all - volume_buy;

                int bufferSize_open_price = snprintf(NULL, 0, "%f", open_price);
                int bufferSize_close_price = snprintf(NULL, 0, "%f", close_price);
                int bufferSize_high_price = snprintf(NULL, 0, "%f", high_price);
                int bufferSize_low_price = snprintf(NULL, 0, "%f", low_price);
                int bufferSize_volume_buy = snprintf(NULL, 0, "%f", volume_buy);
                int bufferSize_volume_sell = snprintf(NULL, 0, "%f", volume_sell);
                char* myString_open_price = (char*)malloc(bufferSize_open_price + 1);
                char* myString_close_price = (char*)malloc(bufferSize_close_price + 1);
                char* myString_high_price = (char*)malloc(bufferSize_high_price + 1);
                char* myString_low_price = (char*)malloc(bufferSize_low_price + 1);
                char* myString_volume_buy = (char*)malloc(bufferSize_volume_buy + 1);
                char* myString_volume_sell = (char*)malloc(bufferSize_volume_sell + 1);
                snprintf(myString_open_price, bufferSize_open_price + 1, "%f", open_price);
                snprintf(myString_close_price, bufferSize_close_price + 1, "%f", close_price);
                snprintf(myString_high_price, bufferSize_high_price + 1, "%f", high_price);
                snprintf(myString_low_price, bufferSize_low_price + 1, "%f", low_price);
                snprintf(myString_volume_buy, bufferSize_volume_buy + 1, "%f", volume_buy);
                snprintf(myString_volume_sell, bufferSize_volume_sell + 1, "%f", volume_sell);
                if (open_price <= close_price) {
                    glColor3ub(14, 203, 129);
                }
                else {
                    glColor3ub(246, 70, 93);
                }
                renderText(myString_open_price, 7 * font_info_width, wnd_size_y - 1 * font_info_height);
                renderText(myString_close_price, 7 * font_info_width, wnd_size_y - 2 * font_info_height);
                renderText(myString_high_price, 26 * font_info_width, wnd_size_y - 1 * font_info_height);
                renderText(myString_low_price, 26 * font_info_width, wnd_size_y - 2 * font_info_height);
                glColor3ub(246, 70, 93);
                renderText(myString_volume_buy, 52 * font_info_width, wnd_size_y - 1 * font_info_height);
                glColor3ub(14, 203, 129);
                renderText(myString_volume_sell, 52 * font_info_width, wnd_size_y - 2 * font_info_height);
                free(myString_open_price);
                free(myString_close_price);
                free(myString_high_price);
                free(myString_low_price);
                free(myString_volume_buy);
                free(myString_volume_sell);
            }
        }
    }
}

void renderTimer(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    UINT64 timestamp
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    unsigned long long time_end_ms = timestamp + 120000;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    unsigned long long time_now_ms = uli.QuadPart / 10000 - 11644473600000;
    long long time_delta_ms = time_end_ms - time_now_ms;
    if (time_delta_ms > 0) {
        char myString[6];
        snprintf(myString, 6, "%2u.%us", time_delta_ms / 1000, (time_delta_ms % 1000) / 100);
        glColor3ub(0, 255, 0);
        renderText(myString, wnd_size_x - 5 * font_time_counter_width, wnd_size_y - 40);
    }
}

void renderChartRsiMfi(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int width_known,
    int size_x,
    int analyse1_config_rows,
    int* analyse1_config,
    int analyse1_data_cols,
    float* analyse1_data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    int amount_known = width_known / size_x;
    float scale = wnd_size_y / 100.0f;

    // draw analyse RSI, MFI
    for (int y = 0; y < analyse1_config_rows; y++) {
        int type = analyse1_config[y * 5 + 0];
        if (type == 6 || type == 7) {
            int len = analyse1_config[y * 5 + 1];
            int color_r = analyse1_config[y * 5 + 2];
            int color_g = analyse1_config[y * 5 + 3];
            int color_b = analyse1_config[y * 5 + 4];
            for (int x = 0; x < amount_known - 1 && x < len - 1; x++) {
                int target_column_2 = len - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = analyse1_data[y * analyse1_data_cols + target_column_1] * scale;
                float y2 = analyse1_data[y * analyse1_data_cols + target_column_2] * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color_r, color_g, color_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    // draw mouse and price
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();

        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = 0.0f;
            value = ((float)mouse_y / (float)wnd_size_y) * 100;

            int bufferSize = snprintf(NULL, 0, "%f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void renderChartStochrsi(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int width_known,
    int size_x,
    int analyse2_config_rows,
    int* analyse2_config,
    int analyse2_data_cols,
    float* analyse2_data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    int amount_known = width_known / size_x;
    float scale = wnd_size_y / 100.0f;

    for (int y = 0; y < analyse2_config_rows; y++) {
        int type = analyse2_config[y * 9 + 0];
        if (type == 8) {
            int len1 = analyse2_config[y * 9 + 1];
            int len2 = analyse2_config[y * 9 + 2];
            int color1_r = analyse2_config[y * 9 + 3];
            int color1_g = analyse2_config[y * 9 + 4];
            int color1_b = analyse2_config[y * 9 + 5];
            int color2_r = analyse2_config[y * 9 + 6];
            int color2_g = analyse2_config[y * 9 + 7];
            int color2_b = analyse2_config[y * 9 + 8];
            for (int x = 0; x < amount_known - 1 && x < len1 - 1; x++) {
                int target_column_2 = len1 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = analyse2_data[(y * 2 + 0) * analyse2_data_cols + target_column_1] * scale;
                float y2 = analyse2_data[(y * 2 + 0) * analyse2_data_cols + target_column_2] * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color1_r, color1_g, color1_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
            for (int x = 0; x < amount_known - 1 && x < len2 - 1; x++) {
                int target_column_2 = len2 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = analyse2_data[(y * 2 + 1) * analyse2_data_cols + target_column_1] * scale;
                float y2 = analyse2_data[(y * 2 + 1) * analyse2_data_cols + target_column_2] * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color2_r, color2_g, color2_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    // draw mouse and price
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();

        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = 0.0f;
            value = ((float)mouse_y / (float)wnd_size_y) * 100;

            int bufferSize = snprintf(NULL, 0, "%f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void renderChartMacd(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int width_known,
    int size_x,
    int analyse3_config_rows,
    int* analyse3_config,
    int analyse3_data_cols,
    float* analyse3_data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    int amount_known = width_known / size_x;
    float maxi = FLT_MIN;
    float mini = FLT_MAX;

    // calculate mini and maxi
    for (int y = 0; y < analyse3_config_rows; y++) {
        int type = analyse3_config[y * 13 + 0];
        if (type == 9) {
            int len1 = analyse3_config[y * 13 + 1];
            int len2 = analyse3_config[y * 13 + 2];
            int len3 = analyse3_config[y * 13 + 3];
            for (int x = 0; x < amount_known && x < len1; x++) {
                int target_column = len1 - x - 1;
                float value = analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
            for (int x = 0; x < amount_known && x < len2; x++) {
                int target_column = len2 - x - 1;
                float value = analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
            for (int x = 0; x < amount_known && x < len3; x++) {
                int target_column = len3 - x - 1;
                float value = analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
        }
    }

    float scale = wnd_size_y / (maxi - mini);

    for (int y = 0; y < analyse3_config_rows; y++) {
        int type = analyse3_config[y * 13 + 0];
        if (type == 9) {
            int len1 = analyse3_config[y * 13 + 1];
            int len2 = analyse3_config[y * 13 + 2];
            int len3 = analyse3_config[y * 13 + 3];
            int color1_r = analyse3_config[y * 13 + 4];
            int color1_g = analyse3_config[y * 13 + 5];
            int color1_b = analyse3_config[y * 13 + 6];
            int color2_r = analyse3_config[y * 13 + 7];
            int color2_g = analyse3_config[y * 13 + 8];
            int color2_b = analyse3_config[y * 13 + 9];
            int color3_r = analyse3_config[y * 13 + 10];
            int color3_g = analyse3_config[y * 13 + 11];
            int color3_b = analyse3_config[y * 13 + 12];
            for (int x = 0; x < amount_known - 1 && x < len1 - 1; x++) {
                int target_column_2 = len1 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;


                glColor3ub(color1_r, color1_g, color1_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
            for (int x = 0; x < amount_known - 1 && x < len2 - 1; x++) {
                int target_column_2 = len2 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color2_r, color2_g, color2_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
            for (int x = 0; x < amount_known - 1 && x < len3 - 1; x++) {
                int target_column_2 = len3 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color3_r, color3_g, color3_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    // draw mouse and price
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();

        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = (((float)mouse_y) / scale) + mini;

            int bufferSize = snprintf(NULL, 0, "%f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void renderChartKdj(
    int wnd_x0,
    int wnd_x1,
    int wnd_y0,
    int wnd_y1,
    int mouse_in_canvas,
    int mouse_y,
    int mouse_x_discret,
    int width_known,
    int size_x,
    int analyse3_config_rows,
    int* analyse3_config,
    int analyse3_data_cols,
    float* analyse3_data
) {
    int wnd_size_x = wnd_x1 - wnd_x0;
    int wnd_size_y = wnd_y1 - wnd_y0;
    mouse_y = mouse_y - wnd_y0;

    glLoadIdentity();
    glViewport(wnd_x0, wnd_y0, wnd_size_x, wnd_size_y);
    glOrtho(0, wnd_size_x, 0, wnd_size_y, -1, 1);

    int amount_known = width_known / size_x;
    float maxi = FLT_MIN;
    float mini = FLT_MAX;

    // calculate mini and maxi
    for (int y = 0; y < analyse3_config_rows; y++) {
        int type = analyse3_config[y * 13 + 0];
        if (type == 10) {
            int len1 = analyse3_config[y * 13 + 1];
            int len2 = analyse3_config[y * 13 + 2];
            int len3 = analyse3_config[y * 13 + 3];
            for (int x = 0; x < amount_known && x < len1; x++) {
                int target_column = len1 - x - 1;
                float value = analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
            for (int x = 0; x < amount_known && x < len2; x++) {
                int target_column = len2 - x - 1;
                float value = analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
            for (int x = 0; x < amount_known && x < len3; x++) {
                int target_column = len3 - x - 1;
                float value = analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column];
                if (value > maxi) {
                    maxi = value;
                }
                if (value < mini) {
                    mini = value;
                }
            }
        }
    }

    float scale = wnd_size_y / (maxi - mini);

    for (int y = 0; y < analyse3_config_rows; y++) {
        int type = analyse3_config[y * 13 + 0];
        if (type == 10) {
            int len1 = analyse3_config[y * 13 + 1];
            int len2 = analyse3_config[y * 13 + 2];
            int len3 = analyse3_config[y * 13 + 3];
            int color1_r = analyse3_config[y * 13 + 4];
            int color1_g = analyse3_config[y * 13 + 5];
            int color1_b = analyse3_config[y * 13 + 6];
            int color2_r = analyse3_config[y * 13 + 7];
            int color2_g = analyse3_config[y * 13 + 8];
            int color2_b = analyse3_config[y * 13 + 9];
            int color3_r = analyse3_config[y * 13 + 10];
            int color3_g = analyse3_config[y * 13 + 11];
            int color3_b = analyse3_config[y * 13 + 12];
            for (int x = 0; x < amount_known - 1 && x < len1 - 1; x++) {
                int target_column_2 = len1 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 0) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color1_r, color1_g, color1_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
            for (int x = 0; x < amount_known - 1 && x < len2 - 1; x++) {
                int target_column_2 = len2 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 1) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color2_r, color2_g, color2_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
            for (int x = 0; x < amount_known - 1 && x < len3 - 1; x++) {
                int target_column_2 = len3 - x - 1;
                int target_column_1 = target_column_2 - 1;
                float y1 = (analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column_1] - mini) * scale;
                float y2 = (analyse3_data[(y * 3 + 2) * analyse3_data_cols + target_column_2] - mini) * scale;
                int x2 = width_known - x * size_x - size_x / 2;
                int x1 = x2 - size_x;

                glColor3ub(color3_r, color3_g, color3_b);
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    // draw mouse and price
    if (mouse_in_canvas == 1) {
        glColor3ub(94, 101, 112);
        glBegin(GL_LINES);
        glVertex2i(mouse_x_discret, 0);
        glVertex2i(mouse_x_discret, wnd_size_y);
        glEnd();

        if (mouse_y >= 0 && mouse_y <= wnd_size_y) {
            glColor3ub(94, 101, 112);
            glBegin(GL_LINES);
            glVertex2i(0, mouse_y);
            glVertex2i(wnd_size_x, mouse_y);
            glEnd();

            float value = (((float)mouse_y) / scale) + mini;

            int bufferSize = snprintf(NULL, 0, "%f", value);
            char* myString = (char*)malloc(bufferSize + 1);
            snprintf(myString, bufferSize + 1, "%f", value);
            glColor3ub(94, 101, 112);
            renderText(myString, wnd_x1 - bufferSize * font_info_width, mouse_y + 1);
            free(myString);
        }
    }
}

void Redraw(
    UINT32 width,
    UINT32 height,

    UINT32 mouse_in_canvas,
    UINT32 mouse_x,
    UINT32 mouse_y,
    UINT32 size_x,

    UINT64 timestamp,

    UINT32 data_rows,
    FLOAT* data,

    UINT32 analyse1_config_rows,
    UINT32* analyse1_config,
    UINT32 analyse1_data_cols,
    FLOAT* analyse1_data,

    UINT32 analyse2_config_rows,
    UINT32* analyse2_config,
    UINT32 analyse2_data_cols,
    FLOAT* analyse2_data,

    UINT32 analyse3_config_rows,
    UINT32* analyse3_config,
    UINT32 analyse3_data_cols,
    FLOAT* analyse3_data,

    UINT32 nn_active,
    FLOAT nn_selected_value,
    UINT32 nn_selected_label,
    FLOAT nn_btc_value,
    UINT32 nn_btc_label,
    FLOAT nn_eth_value,
    UINT32 nn_eth_label,
    FLOAT nn_ltc_value,
    UINT32 nn_ltc_label,
    UINT32 nn_r,
    UINT32 nn_g,
    UINT32 nn_b
) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    wglMakeCurrent(hdc, hglrc);

    glLoadIdentity();
    glViewport(0, 0, width, height);
    glOrtho(0, width, 0, height, -1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    int width_unknown = 300;
    int width_known = width - width_unknown;
    mouse_y = height - mouse_y;

    int additional_charts = 0;
    int additional_chart_rsi_mfi = 0;
    int additional_chart_stochrsi = 0;
    int additional_chart_macd = 0;
    int additional_chart_kdj = 0;
    for (int x = 0; x < analyse1_config_rows; x++) {
        int type = analyse1_config[x * 5 + 0];
        if (type == 6 || type == 7) {
            additional_chart_rsi_mfi++;
        }
    }
    for (int x = 0; x < analyse2_config_rows; x++) {
        int type = analyse2_config[x * 9 + 0];
        if (type == 8) {
            additional_chart_stochrsi++;
        }
    }
    for (int x = 0; x < analyse3_config_rows; x++) {
        int type = analyse3_config[x * 13 + 0];
        if (type == 9) {
            additional_chart_macd++;
        }
        else if (type == 10) {
            additional_chart_kdj++;
        }
    }
    if (additional_chart_rsi_mfi > 0) {
        additional_charts++;
    }
    if (additional_chart_stochrsi > 0) {
        additional_charts++;
    }
    if (additional_chart_macd > 0) {
        additional_charts++;
    }
    if (additional_chart_kdj > 0) {
        additional_charts++;
    }

    int mouse_idx = -1;
    int mouse_x_discret = -1;
    if (mouse_in_canvas == 1) {
        mouse_idx = (mouse_x - (width_known % size_x)) / size_x;
        mouse_x_discret = mouse_idx * size_x + (width_known % size_x) + size_x / 2;
    }

    SelectObject(hdc, hFont_time);
    wglUseFontBitmaps(hdc, 32, 96, fontList);
    renderTimeline(0, width, 0, 10, width_known, size_x, timestamp);

    SelectObject(hdc, hFont_info);
    wglUseFontBitmaps(hdc, 32, 96, fontList);
    renderVolumes(0, width, 10, 110, mouse_in_canvas, mouse_y, mouse_x_discret, width_known, size_x, data_rows, data);

    SelectObject(hdc, hFont_info);
    wglUseFontBitmaps(hdc, 32, 96, fontList);
    renderInfo(0, width, height - 50, height, mouse_in_canvas, mouse_idx, width_known, size_x, data_rows, data);

    SelectObject(hdc, hFont_time_counter);
    wglUseFontBitmaps(hdc, 32, 96, fontList);
    renderTimer(0, width, height - 50, height, timestamp);

    // auto resize
    int chart_candle_height = height - 170;
    int chart_indicator_height = 0;
    if (additional_charts == 1) {
        chart_indicator_height = (2.0f / 7.0f) * chart_candle_height;
        chart_candle_height = chart_candle_height - chart_indicator_height;
    }
    else if (additional_charts == 2) {
        chart_indicator_height = (1.5f / 7.0f) * chart_candle_height;
        chart_candle_height = chart_candle_height - chart_indicator_height;
    }
    else if (additional_charts == 3) {
        chart_indicator_height = (1.333f / 7.0f) * chart_candle_height;
        chart_candle_height = chart_candle_height - chart_indicator_height;
    }
    else if (additional_charts == 4) {
        chart_indicator_height = (1.0f / 7.0f) * chart_candle_height;
        chart_candle_height = chart_candle_height - chart_indicator_height;
    }

    int start_height = 120;
    if (additional_chart_rsi_mfi > 0) {
        SelectObject(hdc, hFont_info);
        wglUseFontBitmaps(hdc, 32, 96, fontList);
        renderChartRsiMfi(0, width, start_height, start_height + chart_indicator_height - 5, mouse_in_canvas, mouse_y, mouse_x_discret, width_known, size_x, analyse1_config_rows, analyse1_config, analyse1_data_cols, analyse1_data);
        start_height += chart_indicator_height;
    }
    if (additional_chart_stochrsi > 0) {
        SelectObject(hdc, hFont_info);
        wglUseFontBitmaps(hdc, 32, 96, fontList);
        renderChartStochrsi(0, width, start_height, start_height + chart_indicator_height - 5, mouse_in_canvas, mouse_y, mouse_x_discret, width_known, size_x, analyse2_config_rows, analyse2_config, analyse2_data_cols, analyse2_data);
        start_height += chart_indicator_height;
    }
    if (additional_chart_macd > 0) {
        SelectObject(hdc, hFont_info);
        wglUseFontBitmaps(hdc, 32, 96, fontList);
        renderChartMacd(0, width, start_height, start_height + chart_indicator_height - 5, mouse_in_canvas, mouse_y, mouse_x_discret, width_known, size_x, analyse3_config_rows, analyse3_config, analyse3_data_cols, analyse3_data);
        start_height += chart_indicator_height;
    }
    if (additional_chart_kdj > 0) {
        SelectObject(hdc, hFont_info);
        wglUseFontBitmaps(hdc, 32, 96, fontList);
        renderChartKdj(0, width, start_height, start_height + chart_indicator_height - 5, mouse_in_canvas, mouse_y, mouse_x_discret, width_known, size_x, analyse3_config_rows, analyse3_config, analyse3_data_cols, analyse3_data);
        start_height += chart_indicator_height;
    }
    SelectObject(hdc, hFont_info);
    wglUseFontBitmaps(hdc, 32, 96, fontList);
    renderGraph(0, width, start_height, height - 50, mouse_in_canvas, mouse_y, mouse_x_discret, mouse_idx, width_known, size_x, data_rows, data, analyse1_config_rows, analyse1_config, analyse1_data_cols, analyse1_data, nn_active, nn_selected_value, nn_selected_label, nn_btc_value, nn_btc_label, nn_eth_value, nn_eth_label, nn_ltc_value, nn_ltc_label, nn_r, nn_g, nn_b);

    glFlush();
    EndPaint(hWnd, &ps);
    ReleaseDC(hWnd, hdc);
}

void Destructor() {
	DeleteObject(hFont_info);
	DeleteObject(hFont_time);
}
