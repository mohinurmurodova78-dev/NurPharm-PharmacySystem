#pragma once

namespace NurPharm {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;

	// ================================================================
	//  Отдельная форма для корзины 
	// ================================================================
	public ref class CartForm : public System::Windows::Forms::Form
	{
	public:
		DataTable^ cartTable;
		DataTable^ medicineTable;
		bool       orderPlaced;

		CartForm(DataTable^ cart, DataTable^ medicine)
		{
			cartTable = cart;
			medicineTable = medicine;
			orderPlaced = false;
			InitCart();
		}

	private:
		System::Windows::Forms::DataGridView^ dgv;
		System::Windows::Forms::Label^ lblTotal;
		System::Windows::Forms::Button^ btnOform;
		System::Windows::Forms::Button^ btnClear;
		System::Windows::Forms::Button^ btnClose;

		void InitCart()
		{
			this->Text = L"Корзина — оформление заказа";
			this->Size = Drawing::Size(550, 480);
			this->StartPosition = FormStartPosition::CenterParent;
			this->BackColor = Color::White;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;

			dgv = gcnew DataGridView();
			dgv->Location = Drawing::Point(10, 10);
			dgv->Size = Drawing::Size(515, 300);
			dgv->DataSource = cartTable;
			dgv->ReadOnly = true;
			dgv->AllowUserToAddRows = false;
			dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgv->BackgroundColor = Color::White;
			dgv->BorderStyle = System::Windows::Forms::BorderStyle::None;
			dgv->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(44, 62, 80);
			dgv->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgv->ColumnHeadersDefaultCellStyle->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			dgv->EnableHeadersVisualStyles = false;

			lblTotal = gcnew Label();
			lblTotal->Font = gcnew Drawing::Font(L"Segoe UI", 11, FontStyle::Bold);
			lblTotal->ForeColor = Color::FromArgb(231, 76, 60);
			lblTotal->Location = Drawing::Point(10, 320);
			lblTotal->Size = Drawing::Size(400, 30);
			UpdateTotal();

			btnOform = gcnew Button();
			btnOform->Text = L"Оформить заказ";
			btnOform->Font = gcnew Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
			btnOform->BackColor = Color::FromArgb(46, 204, 113);
			btnOform->ForeColor = Color::White;
			btnOform->FlatStyle = FlatStyle::Flat;
			btnOform->Location = Drawing::Point(10, 360);
			btnOform->Size = Drawing::Size(200, 40);
			btnOform->UseVisualStyleBackColor = false;
			btnOform->Click += gcnew EventHandler(this, &CartForm::btnOform_Click);

			btnClear = gcnew Button();
			btnClear->Text = L"Очистить корзину";
			btnClear->FlatStyle = FlatStyle::Flat;
			btnClear->Location = Drawing::Point(220, 360);
			btnClear->Size = Drawing::Size(160, 40);
			btnClear->Click += gcnew EventHandler(this, &CartForm::btnClear_Click);

			btnClose = gcnew Button();
			btnClose->Text = L"Закрыть";
			btnClose->FlatStyle = FlatStyle::Flat;
			btnClose->Location = Drawing::Point(390, 360);
			btnClose->Size = Drawing::Size(130, 40);
			btnClose->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->CancelButton = btnClose;

			this->Controls->Add(dgv);
			this->Controls->Add(lblTotal);
			this->Controls->Add(btnOform);
			this->Controls->Add(btnClear);
			this->Controls->Add(btnClose);
		}

		void UpdateTotal()
		{
			Decimal grand = Decimal(0);
			for each(DataRow ^ r in cartTable->Rows)
				grand = Decimal::Add(grand, Convert::ToDecimal(r[L"Итого"]));
			lblTotal->Text = String::Format(L"ИТОГО К ОПЛАТЕ: {0:F2} сум", grand);
		}

		System::Void btnOform_Click(Object^ sender, EventArgs^ e)
		{
			String^ ошибки = L"";
			for each(DataRow ^ cr in cartTable->Rows) {
				String^ cname = cr[L"Name"]->ToString();
				int cqty = Convert::ToInt32(cr[L"Quantity"]);
				for each(DataRow ^ mr in medicineTable->Rows) {
					if (mr->RowState != DataRowState::Deleted && mr[L"Name"]->ToString() == cname) {
						if (Convert::ToInt32(mr[L"Quantity"]) < cqty)
							ошибки += L"Недостаточно: " + cname + L"\n";
					}
				}
			}
			if (ошибки->Length > 0) {
				MessageBox::Show(ошибки, L"Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
				return;
			}

			Decimal grand = Decimal(0);
			for each(DataRow ^ cr in cartTable->Rows) {
				String^ cname = cr[L"Name"]->ToString();
				int cqty = Convert::ToInt32(cr[L"Quantity"]);
				grand = Decimal::Add(grand, Convert::ToDecimal(cr[L"Итого"]));
				for each(DataRow ^ mr in medicineTable->Rows) {
					if (mr->RowState != DataRowState::Deleted && mr[L"Name"]->ToString() == cname) {
						mr[L"Quantity"] = Convert::ToInt32(mr[L"Quantity"]) - cqty;
						break;
					}
				}
			}

			String^ чекТекст =
				L"================================\n"
				L"        NURPHARM АПТЕКА\n"
				L"================================\n"
				L"Дата: " + DateTime::Now.ToString(L"dd.MM.yyyy HH:mm") + L"\n"
				L"--------------------------------\n";

			for each(DataRow ^ cr in cartTable->Rows) {
				чекТекст += String::Format(L"{0}\n  {1} шт. x {2:F2} = {3:F2} сум\n",
					cr[L"Name"], cr[L"Quantity"], cr[L"Price"], cr[L"Итого"]);
			}
			чекТекст +=
				L"--------------------------------\n" +
				String::Format(L"ИТОГО: {0:F2} сум\n", grand) +
				L"================================\n"
				L"     Спасибо за покупку!\n"
				L"================================";

			MessageBox::Show(чекТекст, L"Чек", MessageBoxButtons::OK, MessageBoxIcon::Information);
			cartTable->Rows->Clear();
			orderPlaced = true;
			this->Close();
		}

		System::Void btnClear_Click(Object^ sender, EventArgs^ e)
		{
			if (MessageBox::Show(L"Очистить корзину?", L"Подтверждение",
				MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) {
				cartTable->Rows->Clear();
				this->Close();
			}
		}
	};

	// ================================================================
	//  Главная форма NurPharm
	// ================================================================
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			this->SetStyle(ControlStyles::AllPaintingInWmPaint |
				ControlStyles::UserPaint |
				ControlStyles::DoubleBuffer, true);
			this->UpdateStyles();

			medicineTable = gcnew DataTable(L"Inventory");
			medicineTable->Columns->Add(L"Name", String::typeid);
			medicineTable->Columns->Add(L"Quantity", Int32::typeid);
			medicineTable->Columns->Add(L"Price", Decimal::typeid);
			medicineTable->Columns->Add(L"ImagePath", String::typeid);
			medicineTable->Columns->Add(L"ExpiryDate", DateTime::typeid);
			medicineTable->Columns->Add(L"Category", String::typeid);
			medicineTable->Columns->Add(L"AgeGroup", String::typeid);
			medicineTable->Columns->Add(L"Prescription", String::typeid);

			usersTable = gcnew DataTable(L"Users");
			usersTable->Columns->Add(L"Username", String::typeid);
			usersTable->Columns->Add(L"Password", String::typeid);

			cartTable = gcnew DataTable(L"Cart");
			cartTable->Columns->Add(L"Name", String::typeid);
			cartTable->Columns->Add(L"Quantity", Int32::typeid);
			cartTable->Columns->Add(L"Price", Decimal::typeid);
			cartTable->Columns->Add(L"Итого", Decimal::typeid);

			historyTable = gcnew DataTable(L"History");
			historyTable->Columns->Add(L"DateTime", DateTime::typeid);
			historyTable->Columns->Add(L"Action", String::typeid);
			historyTable->Columns->Add(L"Medicine", String::typeid);
			historyTable->Columns->Add(L"Quantity", Int32::typeid);
			historyTable->Columns->Add(L"Amount", Decimal::typeid);

			InitializeToolTips();

			this->panelMain->Visible = false;
			this->panelLogin->Visible = true;
		}

	protected:
		~MyForm() { if (components) delete components; }

	private:
		DataTable^ medicineTable;
		DataTable^ usersTable;
		DataTable^ cartTable;
		DataTable^ historyTable;
		String^ selectedMedicine = L"";
		String^ selectedImagePath = L"";
		ToolTip^ toolTip;
		int currentPage = 0;
		const int PAGE_SIZE = 20;

		System::Windows::Forms::Panel^ panelLogin;
		System::Windows::Forms::Panel^ panelMain;
		System::Windows::Forms::Panel^ panelSidebar;
		System::Windows::Forms::Panel^ panelHeader;

		System::Windows::Forms::TextBox^ txtUser;
		System::Windows::Forms::TextBox^ txtPass;
		System::Windows::Forms::Button^ btnDoLogin;
		System::Windows::Forms::Button^ btnDoRegister;

		System::Windows::Forms::Label^ lblSearch;
		System::Windows::Forms::TextBox^ txtSearch;
		System::Windows::Forms::ComboBox^ cmbCatFilter;
		System::Windows::Forms::ComboBox^ cmbExpiryFilter;
		System::Windows::Forms::ComboBox^ cmbAgeFilter;
		System::Windows::Forms::ComboBox^ cmbPrescriptionFilter;
		System::Windows::Forms::ComboBox^ cmbSortBy;
		System::Windows::Forms::Button^ btnExport;
		System::Windows::Forms::Button^ btnImport;
		System::Windows::Forms::Button^ btnLogout;
		System::Windows::Forms::Button^ btnShowCart;
		System::Windows::Forms::Button^ btnHistory;
		System::Windows::Forms::Button^ btnFindAnalogs;
		System::Windows::Forms::Button^ btnPrevPage;
		System::Windows::Forms::Button^ btnNextPage;
		System::Windows::Forms::Label^ lblPageInfo;
		System::Windows::Forms::Label^ lblStats;
		System::Windows::Forms::Label^ lblNoResults;

		System::Windows::Forms::FlowLayoutPanel^ flowInventory;

		System::Windows::Forms::PictureBox^ picMedicine;
		System::Windows::Forms::Button^ btnSelectImage;
		System::Windows::Forms::Label^ lblNameLabel;
		System::Windows::Forms::TextBox^ txtName;
		System::Windows::Forms::Label^ lblCatLabel;
		System::Windows::Forms::ComboBox^ cmbCategory;
		System::Windows::Forms::Label^ lblAgeGroupLabel;
		System::Windows::Forms::ComboBox^ cmbAgeGroup;
		System::Windows::Forms::Label^ lblPrescriptionLabel;
		System::Windows::Forms::ComboBox^ cmbPrescription;
		System::Windows::Forms::Label^ lblQtyLabel;
		System::Windows::Forms::NumericUpDown^ numQty;
		System::Windows::Forms::Label^ lblPriceLabel;
		System::Windows::Forms::NumericUpDown^ numPrice;
		System::Windows::Forms::Label^ lblExpiryLabel;
		System::Windows::Forms::DateTimePicker^ dtpExpiry;
		System::Windows::Forms::Button^ btnAdd;
		System::Windows::Forms::Button^ btnAddToCart;
		System::Windows::Forms::Button^ btnSell;
		System::Windows::Forms::Button^ btnRestock;
		System::Windows::Forms::Button^ btnDelete;
		System::Windows::Forms::Button^ btnDeleteExpired;

		System::Collections::Generic::List<String^>^ selectedForDelete =
			gcnew System::Collections::Generic::List<String^>();

		System::ComponentModel::Container^ components;

		Color GetCategoryLabelColor(String^ cat) {
			if (cat->Contains(L"Таблетки")) return Color::FromArgb(46, 125, 50);
			if (cat->Contains(L"Растворы")) return Color::FromArgb(21, 101, 192);
			if (cat->Contains(L"Перевязочные")) return Color::FromArgb(230, 81, 0);
			if (cat->Contains(L"Ампулы")) return Color::FromArgb(194, 24, 91);
			if (cat->Contains(L"Шприцы")) return Color::FromArgb(136, 14, 79);
			if (cat->Contains(L"Сиропы")) return Color::FromArgb(106, 27, 154);
			if (cat->Contains(L"Мази")) return Color::FromArgb(245, 124, 0);
			if (cat->Contains(L"Витамины")) return Color::FromArgb(0, 121, 107);
			if (cat->Contains(L"БАДы")) return Color::FromArgb(93, 64, 55);
			if (cat->Contains(L"Гомеопатия")) return Color::FromArgb(69, 90, 100);
			if (cat->Contains(L"Капельницы")) return Color::FromArgb(40, 53, 147);
			return Color::FromArgb(52, 152, 219);
		}

		void InitializeToolTips()
		{
			toolTip = gcnew ToolTip();
			toolTip->SetToolTip(btnAdd, L"Добавить новый препарат или обновить существующий");
			toolTip->SetToolTip(btnAddToCart, L"Добавить выбранный препарат в корзину");
			toolTip->SetToolTip(btnSell, L"Продать выбранное количество препарата");
			toolTip->SetToolTip(btnRestock, L"Пополнить запасы выбранного препарата");
			toolTip->SetToolTip(btnDelete, L"Удалить выбранные препараты");
			toolTip->SetToolTip(btnDeleteExpired, L"Удалить все просроченные препараты");
			toolTip->SetToolTip(btnSelectImage, L"Выбрать изображение для препарата");
			toolTip->SetToolTip(btnShowCart, L"Открыть корзину для оформления заказа");
			toolTip->SetToolTip(btnExport, L"Экспортировать данные склада в CSV файл");
			toolTip->SetToolTip(btnImport, L"Импортировать данные из CSV файла");
			toolTip->SetToolTip(btnHistory, L"Показать историю операций");
			toolTip->SetToolTip(btnFindAnalogs, L"Найти похожие препараты");
			toolTip->SetToolTip(cmbPrescriptionFilter, L"Фильтр по рецепту/без рецепта");
		}

		void AddMedicineHelper(String^ nm, int q, double p, String^ c, String^ age, String^ presc, int yr, int mo, int dy)
		{
			DataRow^ row = medicineTable->NewRow();
			row[L"Name"] = nm;
			row[L"Quantity"] = q;
			row[L"Price"] = p;
			row[L"Category"] = c;
			row[L"AgeGroup"] = age;
			row[L"Prescription"] = presc;
			row[L"ExpiryDate"] = DateTime(yr, mo, dy);
			row[L"ImagePath"] = L"";
			medicineTable->Rows->Add(row);
		}

		void AddHistory(String^ action, String^ medicine, int qty, Decimal amount)
		{
			DataRow^ hr = historyTable->NewRow();
			hr[L"DateTime"] = DateTime::Now;
			hr[L"Action"] = action;
			hr[L"Medicine"] = medicine;
			hr[L"Quantity"] = qty;
			hr[L"Amount"] = amount;
			historyTable->Rows->Add(hr);
		}

		void CheckLowStock()
		{
			String^ expiredList = L"";      // Список просроченных
			String^ soonExpiredList = L"";  // Список истекающих
			String^ criticalList = L"";     // Список критических (≤ 3)
			String^ lowList = L"";          // Список малых (4–10)

			int expiredCount = 0;
			int soonExpiredCount = 0;
			int criticalCount = 0;
			int lowCount = 0;

			DateTime today = DateTime::Now;
			DateTime warn30 = today.AddDays(30);

			for each(DataRow ^ row in medicineTable->Rows) {
				if (row->RowState == DataRowState::Deleted) continue;

				String^ name = row[L"Name"]->ToString();
				int qty = Convert::ToInt32(row[L"Quantity"]);
				DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);

				// Проверка на просроченность
				if (expiry < today) {
					expiredList += L"  • " + name + L" — " + qty + L" шт. (до " +
						expiry.ToString(L"dd.MM.yyyy") + L")\n";
					expiredCount++;
				}
				// Проверка на истекающий срок
				else if (expiry < warn30) {
					soonExpiredList += L"  • " + name + L" — " + qty + L" шт. (до " +
						expiry.ToString(L"dd.MM.yyyy") + L")\n";
					soonExpiredCount++;
				}

				// Проверка на критический остаток
				if (qty <= 3) {
					criticalList += L"  • " + name + L" — " + qty + L" шт.\n";
					criticalCount++;
				}
				// Проверка на малый остаток
				else if (qty <= 10) {
					lowList += L"  • " + name + L" — " + qty + L" шт.\n";
					lowCount++;
				}
			}

			// Формируем итоговое сообщение с группировкой
			String^ warnings = L"";

			if (expiredCount > 0) {
				warnings += L"❌ ПРОСРОЧЕННЫЕ ПРЕПАРАТЫ (" + expiredCount + L"):\n" + expiredList + L"\n";
			}
			if (soonExpiredCount > 0) {
				warnings += L"⏰ ИСТЕКАЮЩИЕ ПРЕПАРАТЫ (" + soonExpiredCount + L"):\n" + soonExpiredList + L"\n";
			}
			if (criticalCount > 0) {
				warnings += L"🔴 КРИТИЧЕСКИЙ ОСТАТОК (" + criticalCount + L"):\n" + criticalList + L"\n";
			}
			if (lowCount > 0) {
				warnings += L"🟡 МАЛЫЙ ОСТАТОК (" + lowCount + L"):\n" + lowList + L"\n";
			}

			if (warnings->Length > 0) {
				MessageBox::Show(warnings,
					String::Format(L"⚠️ Внимание! Просрочено: {0}, Истекает: {1}, Критических: {2}, Малых: {3}",
						expiredCount, soonExpiredCount, criticalCount, lowCount),
					MessageBoxButtons::OK,
					(expiredCount > 0 || criticalCount > 0) ? MessageBoxIcon::Warning : MessageBoxIcon::Information);
			}
		}

		void UpdatePageControls()
		{
			int totalFiltered = GetFilteredCount();
			int totalPages = Math::Max(1, (int)Math::Ceiling((double)totalFiltered / PAGE_SIZE));
			lblPageInfo->Text = String::Format(L"Стр. {0} из {1} (всего: {2})", currentPage + 1, totalPages, totalFiltered);
			btnPrevPage->Enabled = currentPage > 0;
			btnNextPage->Enabled = currentPage < totalPages - 1;
		}

		int GetFilteredCount()
		{
			return GetFilteredAndSortedRows()->Length;
		}

		array<DataRow^>^ GetFilteredAndSortedRows()
		{
			System::Collections::ArrayList^ list = gcnew System::Collections::ArrayList();
			String^ filter = this->txtSearch->Text->ToLower()->Trim();
			String^ catFilter = L"";
			if (this->cmbCatFilter->SelectedIndex > 0) {
				catFilter = this->cmbCatFilter->SelectedItem->ToString();
				int spacePos = catFilter->IndexOf(L" ");
				if (spacePos > 0 && spacePos < 4) catFilter = catFilter->Substring(spacePos + 1);
			}
			DateTime today = DateTime::Now;
			DateTime warn30 = today.AddDays(30);

			for each(DataRow ^ row in medicineTable->Rows) {
				if (row->RowState == DataRowState::Deleted) continue;

				String^ name = row[L"Name"]->ToString();
				String^ cat = row[L"Category"]->ToString();
				String^ age = (row[L"AgeGroup"] != DBNull::Value) ? row[L"AgeGroup"]->ToString() : L"";
				String^ presc = (row[L"Prescription"] != DBNull::Value) ? row[L"Prescription"]->ToString() : L"";
				DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);

				bool nameOk = name->ToLower()->Contains(filter);
				bool catOk = (catFilter == L"") || (cat == catFilter);

				bool expiryOk = true;
				int expiryFilter = this->cmbExpiryFilter->SelectedIndex;
				if (expiryFilter == 1) expiryOk = expiry < today;
				else if (expiryFilter == 2) expiryOk = !(expiry < today) && expiry < warn30;
				else if (expiryFilter == 3) expiryOk = expiry >= warn30;

				bool ageOk = true;
				int ageFilter = this->cmbAgeFilter->SelectedIndex;
				if (ageFilter == 1) ageOk = age->Contains(L"Детский");
				else if (ageFilter == 2) ageOk = age->Contains(L"Взрослый");
				else if (ageFilter == 3) ageOk = age->Contains(L"60+");
				else if (ageFilter == 4) ageOk = age->Contains(L"Беременные");
				else if (ageFilter == 5) ageOk = age->Contains(L"Кормящие");

				bool prescOk = true;
				int prescFilter = this->cmbPrescriptionFilter->SelectedIndex;
				if (prescFilter == 1) prescOk = presc == L"По рецепту";
				else if (prescFilter == 2) prescOk = presc == L"Без рецепта";

				if (nameOk && catOk && expiryOk && ageOk && prescOk) {
					list->Add(row);
				}
			}

			// Сортировка
			int sortBy = this->cmbSortBy->SelectedIndex;
			for (int i = 0; i < list->Count - 1; i++) {
				for (int j = i + 1; j < list->Count; j++) {
					DataRow^ a = (DataRow^)list[i];
					DataRow^ b = (DataRow^)list[j];
					bool swap = false;
					if (sortBy == 0) swap = String::Compare(a[L"Name"]->ToString(), b[L"Name"]->ToString()) > 0;
					else if (sortBy == 1) swap = Convert::ToDecimal(a[L"Price"]) > Convert::ToDecimal(b[L"Price"]);
					else if (sortBy == 2) swap = Convert::ToInt32(a[L"Quantity"]) > Convert::ToInt32(b[L"Quantity"]);
					else if (sortBy == 3) swap = Convert::ToDateTime(a[L"ExpiryDate"]) > Convert::ToDateTime(b[L"ExpiryDate"]);
					if (swap) {
						Object^ temp = list[i];
						list[i] = list[j];
						list[j] = temp;
					}
				}
			}

			array<DataRow^>^ result = gcnew array<DataRow^>(list->Count);
			list->CopyTo(result);
			return result;
		}

		void UpdateCartButton()
		{
			this->btnShowCart->Text = L"🛒 Корзина (" + cartTable->Rows->Count.ToString() + L")";
		}

		void UpdateStats()
		{
			int totalMeds = 0;
			int expired = 0;
			int soonExpired = 0;
			int lowStock = 0;
			Decimal totalValue = Decimal(0);
			DateTime today = DateTime::Now;
			DateTime warn30 = today.AddDays(30);

			for each(DataRow ^ row in medicineTable->Rows)
			{
				if (row->RowState == DataRowState::Deleted) continue;
				totalMeds++;
				int qty = Convert::ToInt32(row[L"Quantity"]);
				Decimal price = Convert::ToDecimal(row[L"Price"]);
				totalValue = Decimal::Add(totalValue, Decimal::Multiply(price, Decimal(qty)));

				DateTime expiry = (row[L"ExpiryDate"] != DBNull::Value)
					? Convert::ToDateTime(row[L"ExpiryDate"]) : today.AddYears(1);

				if (expiry < today) expired++;
				else if (expiry < warn30) soonExpired++;
				if (qty < 5) lowStock++;
			}

			this->lblStats->Text = String::Format(
				L"📦 Всего: {0}  |  ⚠️ Просрочено: {1}  |  ⏰ Истекает: {2}  |  📉 Мало: {3}  |  💰 Стоимость: {4:F2} сум",
				totalMeds, expired, soonExpired, lowStock, totalValue);
		}

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->panelLogin = gcnew System::Windows::Forms::Panel();
			this->txtUser = gcnew System::Windows::Forms::TextBox();
			this->txtPass = gcnew System::Windows::Forms::TextBox();
			this->btnDoLogin = gcnew System::Windows::Forms::Button();
			this->btnDoRegister = gcnew System::Windows::Forms::Button();

			this->panelMain = gcnew System::Windows::Forms::Panel();
			this->panelHeader = gcnew System::Windows::Forms::Panel();
			this->lblStats = gcnew System::Windows::Forms::Label();
			this->lblSearch = gcnew System::Windows::Forms::Label();
			this->txtSearch = gcnew System::Windows::Forms::TextBox();
			this->cmbCatFilter = gcnew System::Windows::Forms::ComboBox();
			this->cmbExpiryFilter = gcnew System::Windows::Forms::ComboBox();
			this->cmbAgeFilter = gcnew System::Windows::Forms::ComboBox();
			this->cmbPrescriptionFilter = gcnew System::Windows::Forms::ComboBox();
			this->cmbSortBy = gcnew System::Windows::Forms::ComboBox();
			this->btnShowCart = gcnew System::Windows::Forms::Button();
			this->btnHistory = gcnew System::Windows::Forms::Button();
			this->btnExport = gcnew System::Windows::Forms::Button();
			this->btnImport = gcnew System::Windows::Forms::Button();
			this->btnLogout = gcnew System::Windows::Forms::Button();
			this->lblNoResults = gcnew System::Windows::Forms::Label();
			this->flowInventory = gcnew System::Windows::Forms::FlowLayoutPanel();
			this->btnPrevPage = gcnew System::Windows::Forms::Button();
			this->btnNextPage = gcnew System::Windows::Forms::Button();
			this->lblPageInfo = gcnew System::Windows::Forms::Label();

			Panel^ sidebarContainer = gcnew Panel();
			sidebarContainer->Dock = DockStyle::Right;
			sidebarContainer->Width = 270;
			sidebarContainer->BackColor = Color::White;
			sidebarContainer->AutoScroll = true;

			this->panelSidebar = gcnew System::Windows::Forms::Panel();
			this->panelSidebar->BackColor = Color::White;
			this->panelSidebar->Location = Drawing::Point(0, 0);
			this->panelSidebar->Size = Drawing::Size(250, 720);
			this->panelSidebar->Padding = System::Windows::Forms::Padding(12);

			this->picMedicine = gcnew System::Windows::Forms::PictureBox();
			this->btnSelectImage = gcnew System::Windows::Forms::Button();
			this->lblNameLabel = gcnew System::Windows::Forms::Label();
			this->txtName = gcnew System::Windows::Forms::TextBox();
			this->lblCatLabel = gcnew System::Windows::Forms::Label();
			this->cmbCategory = gcnew System::Windows::Forms::ComboBox();
			this->lblAgeGroupLabel = gcnew System::Windows::Forms::Label();
			this->cmbAgeGroup = gcnew System::Windows::Forms::ComboBox();
			this->lblPrescriptionLabel = gcnew System::Windows::Forms::Label();
			this->cmbPrescription = gcnew System::Windows::Forms::ComboBox();
			this->lblQtyLabel = gcnew System::Windows::Forms::Label();
			this->numQty = gcnew System::Windows::Forms::NumericUpDown();
			this->lblPriceLabel = gcnew System::Windows::Forms::Label();
			this->numPrice = gcnew System::Windows::Forms::NumericUpDown();
			this->lblExpiryLabel = gcnew System::Windows::Forms::Label();
			this->dtpExpiry = gcnew System::Windows::Forms::DateTimePicker();
			this->btnAdd = gcnew System::Windows::Forms::Button();
			this->btnAddToCart = gcnew System::Windows::Forms::Button();
			this->btnFindAnalogs = gcnew System::Windows::Forms::Button();
			this->btnSell = gcnew System::Windows::Forms::Button();
			this->btnRestock = gcnew System::Windows::Forms::Button();
			this->btnDelete = gcnew System::Windows::Forms::Button();
			this->btnDeleteExpired = gcnew System::Windows::Forms::Button();

			this->panelLogin->SuspendLayout();
			this->panelMain->SuspendLayout();
			this->panelSidebar->SuspendLayout();
			sidebarContainer->SuspendLayout();
			this->panelHeader->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picMedicine))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numPrice))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numQty))->BeginInit();
			this->SuspendLayout();

			// ==================== ЭКРАН ВХОДА ====================
			this->panelLogin->BackColor = Color::FromArgb(25, 35, 55);
			this->panelLogin->Dock = DockStyle::Fill;

			Panel^ topBar = gcnew Panel();
			topBar->BackColor = Color::FromArgb(20, 28, 45);
			topBar->Dock = DockStyle::Top;
			topBar->Height = 230;

			Label^ logoLabel = gcnew Label();
			logoLabel->Text = L"💊  NURPHARM";
			logoLabel->Font = gcnew Drawing::Font(L"Segoe UI", 42, FontStyle::Bold);
			logoLabel->ForeColor = Color::White;
			logoLabel->Location = Drawing::Point(0, 50);
			logoLabel->Size = Drawing::Size(topBar->Width, 80);
			logoLabel->TextAlign = ContentAlignment::MiddleCenter;
			logoLabel->Anchor = static_cast<AnchorStyles>(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
			logoLabel->AutoSize = false;
			topBar->Controls->Add(logoLabel);

			Label^ subLabel = gcnew Label();
			subLabel->Text = L"Автоматизированная система учета\nпродажи лекарственных средств";
			subLabel->Font = gcnew Drawing::Font(L"Segoe UI", 12, FontStyle::Regular);
			subLabel->ForeColor = Color::FromArgb(180, 190, 210);
			subLabel->Location = Drawing::Point(0, 140);
			subLabel->Size = Drawing::Size(topBar->Width, 55);
			subLabel->TextAlign = ContentAlignment::MiddleCenter;
			subLabel->Anchor = static_cast<AnchorStyles>(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
			subLabel->AutoSize = false;
			topBar->Controls->Add(subLabel);

			this->panelLogin->Controls->Add(topBar);

			Panel^ loginCard = gcnew Panel();
			loginCard->BackColor = Color::White;
			loginCard->Size = Drawing::Size(400, 350);
			loginCard->BorderStyle = BorderStyle::None;
			loginCard->Anchor = AnchorStyles::None;

			Label^ cardTitle = gcnew Label();
			cardTitle->Text = L"Вход в систему";
			cardTitle->Font = gcnew Drawing::Font(L"Segoe UI", 16, FontStyle::Bold);
			cardTitle->ForeColor = Color::FromArgb(44, 62, 80);
			cardTitle->Location = Drawing::Point(30, 20);
			cardTitle->Size = Drawing::Size(340, 35);
			loginCard->Controls->Add(cardTitle);

			Label^ userLabel = gcnew Label();
			userLabel->Text = L"👤  Логин";
			userLabel->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			userLabel->ForeColor = Color::FromArgb(44, 62, 80);
			userLabel->Location = Drawing::Point(30, 70);
			userLabel->Size = Drawing::Size(340, 20);
			loginCard->Controls->Add(userLabel);

			this->txtUser->Font = gcnew Drawing::Font(L"Segoe UI", 11);
			this->txtUser->Location = Drawing::Point(30, 95);
			this->txtUser->Size = Drawing::Size(340, 35);
			this->txtUser->BorderStyle = BorderStyle::FixedSingle;
			this->txtUser->BackColor = Color::FromArgb(248, 249, 250);
			this->txtUser->ForeColor = Color::FromArgb(44, 62, 80);
			loginCard->Controls->Add(this->txtUser);

			Label^ passLabel = gcnew Label();
			passLabel->Text = L"🔒  Пароль";
			passLabel->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			passLabel->ForeColor = Color::FromArgb(44, 62, 80);
			passLabel->Location = Drawing::Point(30, 145);
			passLabel->Size = Drawing::Size(340, 20);
			loginCard->Controls->Add(passLabel);

			this->txtPass->Font = gcnew Drawing::Font(L"Segoe UI", 11);
			this->txtPass->PasswordChar = '*';
			this->txtPass->Location = Drawing::Point(30, 170);
			this->txtPass->Size = Drawing::Size(340, 35);
			this->txtPass->BorderStyle = BorderStyle::FixedSingle;
			this->txtPass->BackColor = Color::FromArgb(248, 249, 250);
			this->txtPass->ForeColor = Color::FromArgb(44, 62, 80);
			loginCard->Controls->Add(this->txtPass);

			this->btnDoLogin->Text = L"🔐  Войти в систему";
			this->btnDoLogin->Font = gcnew Drawing::Font(L"Segoe UI", 11, FontStyle::Bold);
			this->btnDoLogin->BackColor = Color::FromArgb(52, 152, 219);
			this->btnDoLogin->ForeColor = Color::White;
			this->btnDoLogin->FlatStyle = FlatStyle::Flat;
			this->btnDoLogin->FlatAppearance->BorderSize = 0;
			this->btnDoLogin->Location = Drawing::Point(30, 230);
			this->btnDoLogin->Size = Drawing::Size(340, 42);
			this->btnDoLogin->Cursor = Cursors::Hand;
			this->btnDoLogin->UseVisualStyleBackColor = false;
			this->btnDoLogin->Click += gcnew EventHandler(this, &MyForm::btnDoLogin_Click);
			loginCard->Controls->Add(this->btnDoLogin);

			this->btnDoRegister->Text = L"📝  Создать новый аккаунт";
			this->btnDoRegister->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->btnDoRegister->ForeColor = Color::FromArgb(52, 152, 219);
			this->btnDoRegister->FlatStyle = FlatStyle::Flat;
			this->btnDoRegister->FlatAppearance->BorderSize = 0;
			this->btnDoRegister->Location = Drawing::Point(120, 285);
			this->btnDoRegister->Size = Drawing::Size(200, 30);
			this->btnDoRegister->Cursor = Cursors::Hand;
			this->btnDoRegister->Click += gcnew EventHandler(this, &MyForm::btnDoRegister_Click);
			loginCard->Controls->Add(this->btnDoRegister);

			this->panelLogin->Controls->Add(loginCard);
			this->panelLogin->Layout += gcnew LayoutEventHandler(this, &MyForm::CenterLoginCard);

			// ==================== ГЛАВНАЯ ПАНЕЛЬ ====================
			this->panelMain->BackColor = Color::FromArgb(236, 240, 245);
			this->panelMain->Dock = DockStyle::Fill;

			// ==================== ШАПКА ====================
			this->panelHeader->BackColor = Color::FromArgb(44, 62, 80);
			this->panelHeader->Dock = DockStyle::Top;
			this->panelHeader->Height = 180;

			Label^ headerTitle = gcnew Label();
			headerTitle->Text = L"💊  NURPHARM";
			headerTitle->Font = gcnew Drawing::Font(L"Segoe UI", 14, FontStyle::Bold);
			headerTitle->ForeColor = Color::White;
			headerTitle->Location = Drawing::Point(15, 10);
			headerTitle->Size = Drawing::Size(300, 25);
			this->panelHeader->Controls->Add(headerTitle);

			this->btnLogout = gcnew Button();
			this->btnLogout->Text = L"Выйти→";
			this->btnLogout->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->btnLogout->ForeColor = Color::White;
			this->btnLogout->BackColor = Color::FromArgb(231, 76, 60);
			this->btnLogout->FlatStyle = FlatStyle::Flat;
			this->btnLogout->FlatAppearance->BorderSize = 0;
			this->btnLogout->Size = Drawing::Size(70, 30);
			this->btnLogout->Cursor = Cursors::Hand;
			this->btnLogout->UseVisualStyleBackColor = false;
			this->btnLogout->Anchor = (AnchorStyles)(AnchorStyles::Top | AnchorStyles::Right);
			this->btnLogout->Click += gcnew EventHandler(this, &MyForm::btnLogout_Click);
			this->panelHeader->Controls->Add(this->btnLogout);
			this->btnLogout->Location = Drawing::Point(this->panelHeader->Width - 82, 8);

			Panel^ statsPanel = gcnew Panel();
			statsPanel->BackColor = Color::FromArgb(33, 47, 61);
			statsPanel->Location = Drawing::Point(0, 38);
			statsPanel->Size = Drawing::Size(1020, 32);
			statsPanel->Anchor = AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right;

			this->lblStats->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->lblStats->ForeColor = Color::FromArgb(46, 204, 113);
			this->lblStats->Location = Drawing::Point(15, 6);
			this->lblStats->Size = Drawing::Size(950, 20);
			this->lblStats->Text = L"📊 Загрузка...";
			statsPanel->Controls->Add(this->lblStats);
			this->panelHeader->Controls->Add(statsPanel);

			Panel^ searchPanel = gcnew Panel();
			searchPanel->BackColor = Color::FromArgb(52, 73, 94);
			searchPanel->Location = Drawing::Point(0, 70);
			searchPanel->Size = Drawing::Size(1020, 110);
			searchPanel->Anchor = AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right;

			Label^ lblSearchText = gcnew Label();
			lblSearchText->Text = L"Поиск";
			lblSearchText->Font = gcnew Drawing::Font(L"Segoe UI", 7, FontStyle::Bold);
			lblSearchText->ForeColor = Color::FromArgb(200, 200, 200);
			lblSearchText->Location = Drawing::Point(38, 4);
			lblSearchText->Size = Drawing::Size(160, 14);
			searchPanel->Controls->Add(lblSearchText);

			Panel^ searchBox = gcnew Panel();
			searchBox->BackColor = Color::White;
			searchBox->Location = Drawing::Point(12, 20);
			searchBox->Size = Drawing::Size(206, 24);
			searchBox->Padding = System::Windows::Forms::Padding(2);

			this->lblSearch->Text = L"🔍";
			this->lblSearch->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->lblSearch->ForeColor = Color::FromArgb(52, 73, 94);
			this->lblSearch->Location = Drawing::Point(4, 3);
			this->lblSearch->Size = Drawing::Size(18, 18);
			this->lblSearch->TextAlign = ContentAlignment::MiddleCenter;
			searchBox->Controls->Add(this->lblSearch);

			this->txtSearch->Location = Drawing::Point(24, 4);
			this->txtSearch->Size = Drawing::Size(178, 18);
			this->txtSearch->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->txtSearch->BorderStyle = BorderStyle::None;
			this->txtSearch->BackColor = Color::White;
			this->txtSearch->ForeColor = Color::FromArgb(44, 62, 80);
			this->txtSearch->TextChanged += gcnew EventHandler(this, &MyForm::txtSearch_TextChanged);
			searchBox->Controls->Add(this->txtSearch);
			searchPanel->Controls->Add(searchBox);

			array<String^>^ titles = { L"Категория", L"Срок", L"Возраст", L"Рецепт", L"Сорт." };
			array<int>^ xPos = { 228, 348, 468, 588, 708 };

			for (int i = 0; i < titles->Length; i++) {
				Label^ lbl = gcnew Label();
				lbl->Text = titles[i];
				lbl->Font = gcnew Drawing::Font(L"Segoe UI", 7, FontStyle::Bold);
				lbl->ForeColor = Color::FromArgb(200, 200, 200);
				lbl->Location = Drawing::Point(xPos[i], 4);
				lbl->Size = Drawing::Size(80, 14);
				searchPanel->Controls->Add(lbl);
			}

			this->cmbCatFilter->Location = Drawing::Point(228, 20);
			this->cmbCatFilter->Size = Drawing::Size(110, 26);
			this->cmbCatFilter->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbCatFilter->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->cmbCatFilter->BackColor = Color::White;
			this->cmbCatFilter->ForeColor = Color::FromArgb(44, 62, 80);
			this->cmbCatFilter->Items->Clear();
			this->cmbCatFilter->Items->Add(L"📋 Все");
			this->cmbCatFilter->Items->Add(L"💊 Таблетки");
			this->cmbCatFilter->Items->Add(L"🧪 Растворы");
			this->cmbCatFilter->Items->Add(L"🩹 Перевязочные");
			this->cmbCatFilter->Items->Add(L"💉 Ампулы");
			this->cmbCatFilter->Items->Add(L"💉 Шприцы");
			this->cmbCatFilter->Items->Add(L"🥄 Сиропы");
			this->cmbCatFilter->Items->Add(L"🧴 Мази");
			this->cmbCatFilter->Items->Add(L"💊 Витамины");
			this->cmbCatFilter->Items->Add(L"💊 БАДы");
			this->cmbCatFilter->Items->Add(L"🌿 Гомеопатия");
			this->cmbCatFilter->Items->Add(L"💧 Капельницы");
			this->cmbCatFilter->SelectedIndex = 0;
			this->cmbCatFilter->SelectedIndexChanged += gcnew EventHandler(this, &MyForm::cmbCatFilter_Changed);
			searchPanel->Controls->Add(this->cmbCatFilter);

			this->cmbExpiryFilter->Location = Drawing::Point(348, 20);
			this->cmbExpiryFilter->Size = Drawing::Size(110, 26);
			this->cmbExpiryFilter->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbExpiryFilter->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->cmbExpiryFilter->BackColor = Color::White;
			this->cmbExpiryFilter->ForeColor = Color::FromArgb(44, 62, 80);
			this->cmbExpiryFilter->Items->Clear();
			this->cmbExpiryFilter->Items->Add(L"📅 Все сроки");
			this->cmbExpiryFilter->Items->Add(L"⚠️ Просрочено");
			this->cmbExpiryFilter->Items->Add(L"⏰ Истекает");
			this->cmbExpiryFilter->Items->Add(L"✅ Норма");
			this->cmbExpiryFilter->SelectedIndex = 0;
			this->cmbExpiryFilter->SelectedIndexChanged += gcnew EventHandler(this, &MyForm::cmbExpiryFilter_Changed);
			searchPanel->Controls->Add(this->cmbExpiryFilter);

			this->cmbAgeFilter = gcnew ComboBox();
			this->cmbAgeFilter->Location = Drawing::Point(468, 20);
			this->cmbAgeFilter->Size = Drawing::Size(110, 26);
			this->cmbAgeFilter->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbAgeFilter->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->cmbAgeFilter->BackColor = Color::White;
			this->cmbAgeFilter->ForeColor = Color::FromArgb(44, 62, 80);
			this->cmbAgeFilter->Items->Add(L"👤 Все возрасты");
			this->cmbAgeFilter->Items->Add(L"👶 Детский (0-12)");
			this->cmbAgeFilter->Items->Add(L"🧑 Взрослый (12-60)");
			this->cmbAgeFilter->Items->Add(L"👴 60+ (осторожно)");
			this->cmbAgeFilter->Items->Add(L"🤰 Беременные");
			this->cmbAgeFilter->Items->Add(L"🤱 Кормящие");
			this->cmbAgeFilter->SelectedIndex = 0;
			this->cmbAgeFilter->SelectedIndexChanged += gcnew EventHandler(this, &MyForm::cmbAgeFilter_Changed);
			searchPanel->Controls->Add(this->cmbAgeFilter);

			this->cmbPrescriptionFilter = gcnew ComboBox();
			this->cmbPrescriptionFilter->Location = Drawing::Point(588, 20);
			this->cmbPrescriptionFilter->Size = Drawing::Size(110, 26);
			this->cmbPrescriptionFilter->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbPrescriptionFilter->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->cmbPrescriptionFilter->BackColor = Color::White;
			this->cmbPrescriptionFilter->ForeColor = Color::FromArgb(44, 62, 80);
			this->cmbPrescriptionFilter->Items->Add(L"📋 Все");
			this->cmbPrescriptionFilter->Items->Add(L"🔒 По рецепту");
			this->cmbPrescriptionFilter->Items->Add(L"🟢 Без рецепта");
			this->cmbPrescriptionFilter->SelectedIndex = 0;
			this->cmbPrescriptionFilter->SelectedIndexChanged += gcnew EventHandler(this, &MyForm::cmbPrescriptionFilter_Changed);
			searchPanel->Controls->Add(this->cmbPrescriptionFilter);

			this->cmbSortBy = gcnew ComboBox();
			this->cmbSortBy->Location = Drawing::Point(708, 20);
			this->cmbSortBy->Size = Drawing::Size(110, 26);
			this->cmbSortBy->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbSortBy->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->cmbSortBy->BackColor = Color::White;
			this->cmbSortBy->ForeColor = Color::FromArgb(44, 62, 80);
			this->cmbSortBy->Items->Add(L"🔤 Название");
			this->cmbSortBy->Items->Add(L"💰 Цена");
			this->cmbSortBy->Items->Add(L"📦 Кол-во");
			this->cmbSortBy->Items->Add(L"📅 Срок");
			this->cmbSortBy->SelectedIndex = 0;
			this->cmbSortBy->SelectedIndexChanged += gcnew EventHandler(this, &MyForm::cmbSortBy_Changed);
			searchPanel->Controls->Add(this->cmbSortBy);

			int btnY = 55;
			int btnStartX = 12;
			int btnWidth = 102;
			int btnSpacing = 8;

			this->btnShowCart->Text = L"🛒 Корзина (0)";
			this->btnShowCart->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnShowCart->BackColor = Color::FromArgb(255, 152, 0);
			this->btnShowCart->ForeColor = Color::White;
			this->btnShowCart->FlatStyle = FlatStyle::Flat;
			this->btnShowCart->FlatAppearance->BorderSize = 0;
			this->btnShowCart->Location = Drawing::Point(btnStartX, btnY);
			this->btnShowCart->Size = Drawing::Size(btnWidth, 30);
			this->btnShowCart->Cursor = Cursors::Hand;
			this->btnShowCart->UseVisualStyleBackColor = false;
			this->btnShowCart->Click += gcnew EventHandler(this, &MyForm::btnShowCart_Click);
			searchPanel->Controls->Add(this->btnShowCart);

			int histX = btnStartX + btnWidth + btnSpacing;
			this->btnHistory = gcnew Button();
			this->btnHistory->Text = L"📋 История";
			this->btnHistory->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnHistory->BackColor = Color::FromArgb(76, 175, 80);
			this->btnHistory->ForeColor = Color::White;
			this->btnHistory->FlatStyle = FlatStyle::Flat;
			this->btnHistory->FlatAppearance->BorderSize = 0;
			this->btnHistory->Location = Drawing::Point(histX, btnY);
			this->btnHistory->Size = Drawing::Size(btnWidth, 30);
			this->btnHistory->Cursor = Cursors::Hand;
			this->btnHistory->UseVisualStyleBackColor = false;
			this->btnHistory->Click += gcnew EventHandler(this, &MyForm::btnHistory_Click);
			searchPanel->Controls->Add(this->btnHistory);

			int expX = histX + btnWidth + btnSpacing;
			this->btnExport = gcnew Button();
			this->btnExport->Text = L"📥 Экспорт";
			this->btnExport->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnExport->BackColor = Color::FromArgb(156, 39, 176);
			this->btnExport->ForeColor = Color::White;
			this->btnExport->FlatStyle = FlatStyle::Flat;
			this->btnExport->FlatAppearance->BorderSize = 0;
			this->btnExport->Location = Drawing::Point(expX, btnY);
			this->btnExport->Size = Drawing::Size(btnWidth, 30);
			this->btnExport->Cursor = Cursors::Hand;
			this->btnExport->UseVisualStyleBackColor = false;
			this->btnExport->Click += gcnew EventHandler(this, &MyForm::btnExport_Click);
			searchPanel->Controls->Add(this->btnExport);

			int impX = expX + btnWidth + btnSpacing;
			this->btnImport = gcnew Button();
			this->btnImport->Text = L"📤 Импорт";
			this->btnImport->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnImport->BackColor = Color::FromArgb(33, 150, 243);
			this->btnImport->ForeColor = Color::White;
			this->btnImport->FlatStyle = FlatStyle::Flat;
			this->btnImport->FlatAppearance->BorderSize = 0;
			this->btnImport->Location = Drawing::Point(impX, btnY);
			this->btnImport->Size = Drawing::Size(btnWidth, 30);
			this->btnImport->Cursor = Cursors::Hand;
			this->btnImport->UseVisualStyleBackColor = false;
			this->btnImport->Click += gcnew EventHandler(this, &MyForm::btnImport_Click);
			searchPanel->Controls->Add(this->btnImport);
			this->panelHeader->Controls->Add(searchPanel);

			// ==================== КАРТОЧКИ + ПАГИНАЦИЯ ====================
			Panel^ cardsPanel = gcnew Panel();
			cardsPanel->Dock = DockStyle::Fill;
			cardsPanel->BackColor = Color::FromArgb(236, 240, 245);

			this->flowInventory->AutoScroll = true;
			this->flowInventory->Dock = DockStyle::Fill;
			this->flowInventory->Padding = System::Windows::Forms::Padding(20, 15, 20, 15);
			this->flowInventory->BackColor = Color::FromArgb(236, 240, 245);
			this->flowInventory->GetType()->GetProperty(L"DoubleBuffered",
				System::Reflection::BindingFlags::Instance |
				System::Reflection::BindingFlags::NonPublic)
				->SetValue(this->flowInventory, true, nullptr);

			this->lblNoResults = gcnew Label();
			this->lblNoResults->Text = L"🔍 Ничего не найдено";
			this->lblNoResults->Font = gcnew Drawing::Font(L"Segoe UI", 14, FontStyle::Bold);
			this->lblNoResults->ForeColor = Color::FromArgb(150, 150, 150);
			this->lblNoResults->AutoSize = true;
			this->lblNoResults->Visible = false;
			this->flowInventory->Controls->Add(this->lblNoResults);

			// Панель пагинации
			Panel^ paginationPanel = gcnew Panel();
			paginationPanel->Dock = DockStyle::Bottom;
			paginationPanel->Height = 55;
			paginationPanel->BackColor = Color::FromArgb(236, 240, 245);
			paginationPanel->Padding = System::Windows::Forms::Padding(0, 10, 0, 10);

			this->btnPrevPage = gcnew Button();
			this->btnPrevPage->Text = L"◀";
			this->btnPrevPage->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnPrevPage->BackColor = Color::FromArgb(52, 152, 219);
			this->btnPrevPage->ForeColor = Color::White;
			this->btnPrevPage->FlatStyle = FlatStyle::Flat;
			this->btnPrevPage->FlatAppearance->BorderSize = 0;
			this->btnPrevPage->Size = Drawing::Size(40, 30);
			this->btnPrevPage->Cursor = Cursors::Hand;
			this->btnPrevPage->UseVisualStyleBackColor = false;
			this->btnPrevPage->Click += gcnew EventHandler(this, &MyForm::btnPrevPage_Click);

			this->lblPageInfo = gcnew Label();
			this->lblPageInfo->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->lblPageInfo->ForeColor = Color::FromArgb(44, 62, 80);
			this->lblPageInfo->Size = Drawing::Size(150, 30);
			this->lblPageInfo->TextAlign = ContentAlignment::MiddleCenter;
			this->lblPageInfo->Text = L"Стр. 1 из 1";

			this->btnNextPage = gcnew Button();
			this->btnNextPage->Text = L"▶";
			this->btnNextPage->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnNextPage->BackColor = Color::FromArgb(52, 152, 219);
			this->btnNextPage->ForeColor = Color::White;
			this->btnNextPage->FlatStyle = FlatStyle::Flat;
			this->btnNextPage->FlatAppearance->BorderSize = 0;
			this->btnNextPage->Size = Drawing::Size(40, 30);
			this->btnNextPage->Cursor = Cursors::Hand;
			this->btnNextPage->UseVisualStyleBackColor = false;
			this->btnNextPage->Click += gcnew EventHandler(this, &MyForm::btnNextPage_Click);

			paginationPanel->Controls->Add(this->btnPrevPage);
			paginationPanel->Controls->Add(this->lblPageInfo);
			paginationPanel->Controls->Add(this->btnNextPage);
			paginationPanel->Resize += gcnew EventHandler(this, &MyForm::PaginationPanel_Resize);

			cardsPanel->Controls->Add(this->flowInventory);
			cardsPanel->Controls->Add(paginationPanel);

			// ==================== БОКОВАЯ ПАНЕЛЬ ====================
			this->picMedicine->BackColor = Color::FromArgb(248, 249, 250);
			this->picMedicine->BorderStyle = BorderStyle::None;
			this->picMedicine->Location = Drawing::Point(18, 12);
			this->picMedicine->Size = Drawing::Size(214, 100);
			this->picMedicine->SizeMode = PictureBoxSizeMode::Zoom;
			this->picMedicine->TabStop = false;

			this->btnSelectImage->Text = L"📷 Выбрать изображение";
			this->btnSelectImage->Font = gcnew Drawing::Font(L"Segoe UI", 8);
			this->btnSelectImage->BackColor = Color::FromArgb(236, 240, 241);
			this->btnSelectImage->ForeColor = Color::FromArgb(33, 37, 41);
			this->btnSelectImage->FlatStyle = FlatStyle::Flat;
			this->btnSelectImage->FlatAppearance->BorderSize = 0;
			this->btnSelectImage->Location = Drawing::Point(18, 117);
			this->btnSelectImage->Size = Drawing::Size(214, 24);
			this->btnSelectImage->Cursor = Cursors::Hand;
			this->btnSelectImage->UseVisualStyleBackColor = false;
			this->btnSelectImage->Click += gcnew EventHandler(this, &MyForm::btnSelectImage_Click);

			this->lblNameLabel->Text = L"Название препарата";
			this->lblNameLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblNameLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblNameLabel->Location = Drawing::Point(18, 146);
			this->lblNameLabel->Size = Drawing::Size(214, 15);

			this->txtName->Location = Drawing::Point(18, 162);
			this->txtName->Size = Drawing::Size(214, 22);
			this->txtName->BorderStyle = BorderStyle::FixedSingle;
			this->txtName->BackColor = Color::FromArgb(248, 249, 250);

			this->lblCatLabel->Text = L"Категория";
			this->lblCatLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblCatLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblCatLabel->Location = Drawing::Point(18, 188);
			this->lblCatLabel->Size = Drawing::Size(214, 15);

			this->cmbCategory->Location = Drawing::Point(18, 204);
			this->cmbCategory->Size = Drawing::Size(214, 22);
			this->cmbCategory->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbCategory->BackColor = Color::FromArgb(248, 249, 250);
			this->cmbCategory->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->cmbCategory->Items->Add(L"💊 Таблетки");
			this->cmbCategory->Items->Add(L"🧪 Растворы");
			this->cmbCategory->Items->Add(L"🩹 Перевязочные");
			this->cmbCategory->Items->Add(L"💉 Ампулы");
			this->cmbCategory->Items->Add(L"💉 Шприцы");
			this->cmbCategory->Items->Add(L"🥄 Сиропы");
			this->cmbCategory->Items->Add(L"🧴 Мази");
			this->cmbCategory->Items->Add(L"💊 Витамины");
			this->cmbCategory->Items->Add(L"💊 БАДы");
			this->cmbCategory->Items->Add(L"🌿 Гомеопатия");
			this->cmbCategory->Items->Add(L"💧 Капельницы");
			this->cmbCategory->SelectedIndex = 0;

			this->lblAgeGroupLabel = gcnew Label();
			this->lblAgeGroupLabel->Text = L"Возрастная группа";
			this->lblAgeGroupLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblAgeGroupLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblAgeGroupLabel->Location = Drawing::Point(18, 230);
			this->lblAgeGroupLabel->Size = Drawing::Size(214, 15);

			this->cmbAgeGroup = gcnew ComboBox();
			this->cmbAgeGroup->Location = Drawing::Point(18, 246);
			this->cmbAgeGroup->Size = Drawing::Size(214, 22);
			this->cmbAgeGroup->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbAgeGroup->BackColor = Color::FromArgb(248, 249, 250);
			this->cmbAgeGroup->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->cmbAgeGroup->Items->Add(L"👶 Детский (0-12)");
			this->cmbAgeGroup->Items->Add(L"🧑 Взрослый (12-60)");
			this->cmbAgeGroup->Items->Add(L"👴 60+ (осторожно)");
			this->cmbAgeGroup->Items->Add(L"🤰 Беременные");
			this->cmbAgeGroup->Items->Add(L"🤱 Кормящие");
			this->cmbAgeGroup->Items->Add(L"👤 Универсальный");
			this->cmbAgeGroup->SelectedIndex = 5;

			this->lblPrescriptionLabel = gcnew Label();
			this->lblPrescriptionLabel->Text = L"Рецепт";
			this->lblPrescriptionLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblPrescriptionLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblPrescriptionLabel->Location = Drawing::Point(18, 272);
			this->lblPrescriptionLabel->Size = Drawing::Size(214, 15);

			this->cmbPrescription = gcnew ComboBox();
			this->cmbPrescription->Location = Drawing::Point(18, 288);
			this->cmbPrescription->Size = Drawing::Size(214, 22);
			this->cmbPrescription->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cmbPrescription->BackColor = Color::FromArgb(248, 249, 250);
			this->cmbPrescription->Font = gcnew Drawing::Font(L"Segoe UI", 9);
			this->cmbPrescription->Items->Add(L"🟢 Без рецепта");
			this->cmbPrescription->Items->Add(L"🔒 По рецепту");
			this->cmbPrescription->SelectedIndex = 0;

			this->lblQtyLabel->Text = L"Количество (шт.)";
			this->lblQtyLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblQtyLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblQtyLabel->Location = Drawing::Point(18, 314);
			this->lblQtyLabel->Size = Drawing::Size(214, 15);

			this->numQty->Location = Drawing::Point(18, 330);
			this->numQty->Size = Drawing::Size(100, 22);
			this->numQty->BackColor = Color::FromArgb(248, 249, 250);
			this->numQty->Maximum = Decimal(gcnew cli::array<Int32>(4) { 100000, 0, 0, 0 });

			this->lblPriceLabel->Text = L"Цена (сум)";
			this->lblPriceLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblPriceLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblPriceLabel->Location = Drawing::Point(18, 356);
			this->lblPriceLabel->Size = Drawing::Size(214, 15);

			this->numPrice->DecimalPlaces = 2;
			this->numPrice->Location = Drawing::Point(18, 372);
			this->numPrice->Size = Drawing::Size(120, 22);
			this->numPrice->BackColor = Color::FromArgb(248, 249, 250);
			this->numPrice->Maximum = Decimal(gcnew cli::array<Int32>(4) { 10000000, 0, 0, 0 });

			this->lblExpiryLabel->Text = L"Срок годности";
			this->lblExpiryLabel->Font = gcnew Drawing::Font(L"Segoe UI", 8, FontStyle::Bold);
			this->lblExpiryLabel->ForeColor = Color::FromArgb(108, 117, 125);
			this->lblExpiryLabel->Location = Drawing::Point(18, 398);
			this->lblExpiryLabel->Size = Drawing::Size(214, 15);

			this->dtpExpiry->Format = DateTimePickerFormat::Short;
			this->dtpExpiry->Location = Drawing::Point(18, 414);
			this->dtpExpiry->Size = Drawing::Size(214, 22);

			this->btnAdd->Text = L"✅ Добавить / Обновить";
			this->btnAdd->BackColor = Color::FromArgb(46, 204, 113);
			this->btnAdd->ForeColor = Color::White;
			this->btnAdd->FlatStyle = FlatStyle::Flat;
			this->btnAdd->FlatAppearance->BorderSize = 0;
			this->btnAdd->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnAdd->Location = Drawing::Point(18, 444);
			this->btnAdd->Size = Drawing::Size(214, 30);
			this->btnAdd->Cursor = Cursors::Hand;
			this->btnAdd->UseVisualStyleBackColor = false;
			this->btnAdd->Click += gcnew EventHandler(this, &MyForm::btnAdd_Click);

			this->btnAddToCart->Text = L"🛒 В корзину";
			this->btnAddToCart->BackColor = Color::FromArgb(255, 152, 0);
			this->btnAddToCart->ForeColor = Color::White;
			this->btnAddToCart->FlatStyle = FlatStyle::Flat;
			this->btnAddToCart->FlatAppearance->BorderSize = 0;
			this->btnAddToCart->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnAddToCart->Location = Drawing::Point(18, 478);
			this->btnAddToCart->Size = Drawing::Size(214, 30);
			this->btnAddToCart->Cursor = Cursors::Hand;
			this->btnAddToCart->UseVisualStyleBackColor = false;
			this->btnAddToCart->Click += gcnew EventHandler(this, &MyForm::btnAddToCart_Click);

			this->btnFindAnalogs->Text = L"🔍 Найти похожие";
			this->btnFindAnalogs->BackColor = Color::FromArgb(0, 150, 136);
			this->btnFindAnalogs->ForeColor = Color::White;
			this->btnFindAnalogs->FlatStyle = FlatStyle::Flat;
			this->btnFindAnalogs->FlatAppearance->BorderSize = 0;
			this->btnFindAnalogs->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnFindAnalogs->Location = Drawing::Point(18, 512);
			this->btnFindAnalogs->Size = Drawing::Size(214, 30);
			this->btnFindAnalogs->Cursor = Cursors::Hand;
			this->btnFindAnalogs->UseVisualStyleBackColor = false;
			this->btnFindAnalogs->Click += gcnew EventHandler(this, &MyForm::btnFindAnalogs_Click);

			this->btnSell->Text = L"💰 Продать";
			this->btnSell->BackColor = Color::FromArgb(244, 67, 54);
			this->btnSell->ForeColor = Color::White;
			this->btnSell->FlatStyle = FlatStyle::Flat;
			this->btnSell->FlatAppearance->BorderSize = 0;
			this->btnSell->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnSell->Location = Drawing::Point(18, 546);
			this->btnSell->Size = Drawing::Size(102, 30);
			this->btnSell->Cursor = Cursors::Hand;
			this->btnSell->UseVisualStyleBackColor = false;
			this->btnSell->Click += gcnew EventHandler(this, &MyForm::btnSell_Click);

			this->btnRestock->Text = L"📦 Пополнить";
			this->btnRestock->BackColor = Color::FromArgb(33, 150, 243);
			this->btnRestock->ForeColor = Color::White;
			this->btnRestock->FlatStyle = FlatStyle::Flat;
			this->btnRestock->FlatAppearance->BorderSize = 0;
			this->btnRestock->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnRestock->Location = Drawing::Point(130, 546);
			this->btnRestock->Size = Drawing::Size(102, 30);
			this->btnRestock->Cursor = Cursors::Hand;
			this->btnRestock->UseVisualStyleBackColor = false;
			this->btnRestock->Click += gcnew EventHandler(this, &MyForm::btnRestock_Click);

			this->btnDelete->Text = L"🗑️ Удалить выбранные";
			this->btnDelete->BackColor = Color::FromArgb(192, 57, 43);
			this->btnDelete->ForeColor = Color::White;
			this->btnDelete->FlatStyle = FlatStyle::Flat;
			this->btnDelete->FlatAppearance->BorderSize = 0;
			this->btnDelete->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnDelete->Location = Drawing::Point(18, 582);
			this->btnDelete->Size = Drawing::Size(214, 28);
			this->btnDelete->Cursor = Cursors::Hand;
			this->btnDelete->UseVisualStyleBackColor = false;
			this->btnDelete->Click += gcnew EventHandler(this, &MyForm::btnDeleteSelected_Click);

			this->btnDeleteExpired = gcnew Button();
			this->btnDeleteExpired->Text = L"⏰ Удалить просроченные";
			this->btnDeleteExpired->BackColor = Color::FromArgb(230, 126, 34);
			this->btnDeleteExpired->ForeColor = Color::White;
			this->btnDeleteExpired->FlatStyle = FlatStyle::Flat;
			this->btnDeleteExpired->FlatAppearance->BorderSize = 0;
			this->btnDeleteExpired->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			this->btnDeleteExpired->Location = Drawing::Point(18, 614);
			this->btnDeleteExpired->Size = Drawing::Size(214, 28);
			this->btnDeleteExpired->Cursor = Cursors::Hand;
			this->btnDeleteExpired->UseVisualStyleBackColor = false;
			this->btnDeleteExpired->Click += gcnew EventHandler(this, &MyForm::btnDeleteExpired_Click);

			// Добавляем элементы в боковую панель
			this->panelSidebar->Controls->Add(this->picMedicine);
			this->panelSidebar->Controls->Add(this->btnSelectImage);
			this->panelSidebar->Controls->Add(this->lblNameLabel);
			this->panelSidebar->Controls->Add(this->txtName);
			this->panelSidebar->Controls->Add(this->lblCatLabel);
			this->panelSidebar->Controls->Add(this->cmbCategory);
			this->panelSidebar->Controls->Add(this->lblAgeGroupLabel);
			this->panelSidebar->Controls->Add(this->cmbAgeGroup);
			this->panelSidebar->Controls->Add(this->lblPrescriptionLabel);
			this->panelSidebar->Controls->Add(this->cmbPrescription);
			this->panelSidebar->Controls->Add(this->lblQtyLabel);
			this->panelSidebar->Controls->Add(this->numQty);
			this->panelSidebar->Controls->Add(this->lblPriceLabel);
			this->panelSidebar->Controls->Add(this->numPrice);
			this->panelSidebar->Controls->Add(this->lblExpiryLabel);
			this->panelSidebar->Controls->Add(this->dtpExpiry);
			this->panelSidebar->Controls->Add(this->btnAdd);
			this->panelSidebar->Controls->Add(this->btnAddToCart);
			this->panelSidebar->Controls->Add(this->btnFindAnalogs);
			this->panelSidebar->Controls->Add(this->btnSell);
			this->panelSidebar->Controls->Add(this->btnRestock);
			this->panelSidebar->Controls->Add(this->btnDelete);
			this->panelSidebar->Controls->Add(this->btnDeleteExpired);

			sidebarContainer->Controls->Add(this->panelSidebar);
			this->panelMain->Controls->Add(cardsPanel);

			this->panelMain->Controls->Add(sidebarContainer);
			this->panelMain->Controls->Add(this->panelHeader);

			this->ClientSize = Drawing::Size(1050, 660);
			this->MinimumSize = Drawing::Size(1050, 700);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->WindowState = FormWindowState::Maximized;
			this->Text = L"NurPharm — Система продажи лекарственных средств";
			this->BackColor = Color::FromArgb(236, 240, 245);
			this->Controls->Add(this->panelMain);
			this->Controls->Add(this->panelLogin);
			this->FormClosing += gcnew FormClosingEventHandler(this, &MyForm::MyForm_FormClosing);
			this->Load += gcnew EventHandler(this, &MyForm::MyForm_Load);

			this->panelLogin->ResumeLayout(false);
			this->panelMain->ResumeLayout(false);
			this->panelSidebar->ResumeLayout(false);
			this->panelSidebar->PerformLayout();
			sidebarContainer->ResumeLayout(false);
			this->panelHeader->ResumeLayout(false);
			this->panelHeader->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picMedicine))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numPrice))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numQty))->EndInit();
			this->ResumeLayout(false);
		}
#pragma endregion

		// ==================== МЕТОДЫ ====================

		void RefreshCards()
		{
			this->flowInventory->Visible = false;
			this->flowInventory->SuspendLayout();
			this->flowInventory->Controls->Clear();
			this->flowInventory->Controls->Add(this->lblNoResults);

			array<DataRow^>^ filteredRows = GetFilteredAndSortedRows();

			Decimal total = Decimal(0);
			DateTime today = DateTime::Now;
			DateTime warn30 = today.AddDays(30);

			int startIdx = currentPage * PAGE_SIZE;
			int endIdx = Math::Min(startIdx + PAGE_SIZE, filteredRows->Length);

			if (filteredRows->Length == 0) {
				this->lblNoResults->Visible = true;
				this->lblNoResults->Location = Drawing::Point(20, 20);
				UpdatePageControls();
				this->flowInventory->ResumeLayout();
				return;
			}
			this->lblNoResults->Visible = false;

			for (int idx = startIdx; idx < endIdx; idx++)
			{
				DataRow^ row = filteredRows[idx];
				String^ name = row[L"Name"]->ToString();
				int qty = Convert::ToInt32(row[L"Quantity"]);
				Decimal price = Convert::ToDecimal(row[L"Price"]);
				String^ imgPath = (row[L"ImagePath"] != DBNull::Value) ? row[L"ImagePath"]->ToString() : L"";
				String^ cat = (row[L"Category"] != DBNull::Value) ? row[L"Category"]->ToString() : L"";
				DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);
				total = Decimal::Add(total, Decimal::Multiply(price, Decimal(qty)));

				Panel^ card = gcnew Panel();
				card->Size = Drawing::Size(260, 150);
				card->Margin = System::Windows::Forms::Padding(6, 6, 6, 16);
				card->Cursor = Cursors::Hand;
				card->Tag = name;
				card->BorderStyle = BorderStyle::FixedSingle;

				CheckBox^ chkSelect = gcnew CheckBox();
				chkSelect->Location = Drawing::Point(232, 5);
				chkSelect->Size = Drawing::Size(22, 22);
				chkSelect->Tag = name;
				chkSelect->BackColor = Color::White;
				chkSelect->FlatStyle = FlatStyle::Flat;
				chkSelect->FlatAppearance->BorderSize = 1;
				chkSelect->FlatAppearance->BorderColor = Color::FromArgb(180, 180, 180);
				chkSelect->FlatAppearance->CheckedBackColor = Color::FromArgb(46, 204, 113);
				chkSelect->Text = L"";
				if (selectedForDelete->Contains(name))
					chkSelect->Checked = true;
				chkSelect->CheckedChanged += gcnew EventHandler(this, &MyForm::ChkSelect_CheckedChanged);
				card->Controls->Add(chkSelect);

				bool isExpired = expiry < today;
				bool isSoon = !isExpired && expiry < warn30;
				if (isExpired) card->BackColor = Color::FromArgb(255, 235, 238);
				else if (isSoon) card->BackColor = Color::FromArgb(255, 253, 231);
				else if (name == selectedMedicine) card->BackColor = Color::FromArgb(227, 242, 253);
				else card->BackColor = Color::White;
				card->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				Panel^ imageContainer = gcnew Panel();
				imageContainer->Size = Drawing::Size(115, 105);
				imageContainer->Location = Drawing::Point(3, 3);
				imageContainer->BackColor = Color::FromArgb(248, 249, 250);
				imageContainer->BorderStyle = BorderStyle::FixedSingle;
				imageContainer->Tag = name;
				imageContainer->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				PictureBox^ pic = gcnew PictureBox();
				pic->Size = Drawing::Size(105, 95);
				pic->Location = Drawing::Point(5, 5);
				pic->SizeMode = PictureBoxSizeMode::Zoom;
				pic->BackColor = Color::Transparent;
				pic->TabStop = false;
				pic->Tag = name;
				if (!String::IsNullOrEmpty(imgPath) && File::Exists(imgPath))
					pic->ImageLocation = imgPath;
				pic->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				imageContainer->Controls->Add(pic);

				int textX = 120;
				int textW = 110;

				Label^ lName = gcnew Label();
				lName->Text = name;
				lName->Font = gcnew Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
				lName->Location = Drawing::Point(textX, 5);
				lName->Size = Drawing::Size(textW, 20);
				lName->Tag = name;
				lName->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				Label^ lCat = gcnew Label();
				lCat->Text = L"  " + cat + L"  ";
				lCat->Font = gcnew Drawing::Font(L"Segoe UI", 7, FontStyle::Bold);
				lCat->ForeColor = Color::White;
				lCat->BackColor = GetCategoryLabelColor(cat);
				lCat->Location = Drawing::Point(textX, 28);
				lCat->Size = Drawing::Size(textW, 18);
				lCat->TextAlign = ContentAlignment::MiddleLeft;
				lCat->Tag = name;
				lCat->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				Label^ lQty = gcnew Label();
				lQty->Text = L"📦 " + qty.ToString() + L" шт.";
				lQty->Font = gcnew Drawing::Font(L"Segoe UI", 8);
				lQty->ForeColor = qty < 5 ? Color::Crimson : Color::FromArgb(60, 60, 60);
				lQty->Location = Drawing::Point(textX, 49);
				lQty->Size = Drawing::Size(textW, 18);
				lQty->Tag = name;
				lQty->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				Label^ lPrice = gcnew Label();
				lPrice->Text = L"💰 " + price.ToString(L"F2") + L" сум";
				lPrice->Font = gcnew Drawing::Font(L"Segoe UI", 8);
				lPrice->ForeColor = Color::FromArgb(60, 60, 60);
				lPrice->Location = Drawing::Point(textX, 67);
				lPrice->Size = Drawing::Size(textW, 18);
				lPrice->Tag = name;
				lPrice->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				String^ expStr = isExpired ? L"⚠️ ПРОСРОЧЕНО!" : expiry.ToString(L"dd.MM.yyyy");
				Label^ lExpiry = gcnew Label();
				lExpiry->Text = L"📅 " + expStr;
				lExpiry->Font = gcnew Drawing::Font(L"Segoe UI", 8);
				lExpiry->ForeColor = isExpired ? Color::Red : (isSoon ? Color::DarkOrange : Color::FromArgb(60, 60, 60));
				lExpiry->Location = Drawing::Point(textX, 85);
				lExpiry->Size = Drawing::Size(textW, 18);
				lExpiry->Tag = name;
				lExpiry->Click += gcnew EventHandler(this, &MyForm::Card_Click);

				String^ presc = (row[L"Prescription"] != DBNull::Value) ? row[L"Prescription"]->ToString() : L"";
				String^ age = (row[L"AgeGroup"] != DBNull::Value) ? row[L"AgeGroup"]->ToString() : L"";
				int bottomY = 105;
				if (presc->Contains(L"По рецепту")) {
					Label^ lPresc = gcnew Label();
					lPresc->Text = L"🔒 Рецепт";
					lPresc->Font = gcnew Drawing::Font(L"Segoe UI", 7, FontStyle::Bold);
					lPresc->ForeColor = Color::White;
					lPresc->BackColor = Color::FromArgb(244, 67, 54);
					lPresc->Location = Drawing::Point(textX, bottomY);
					lPresc->Size = Drawing::Size(55, 16);
					lPresc->TextAlign = ContentAlignment::MiddleCenter;
					card->Controls->Add(lPresc);
				}

				card->Controls->Add(imageContainer);
				card->Controls->Add(lName);
				card->Controls->Add(lCat);
				card->Controls->Add(lQty);
				card->Controls->Add(lPrice);
				card->Controls->Add(lExpiry);
				this->flowInventory->Controls->Add(card);
			}

			UpdateCartButton();
			UpdateStats();
			UpdatePageControls();

			this->flowInventory->ResumeLayout(false);
			this->flowInventory->PerformLayout();
			this->flowInventory->Visible = true;

		}

		System::Void PaginationPanel_Resize(Object^ sender, EventArgs^ e)
		{
			Panel^ p = (Panel^)sender;
			this->btnPrevPage->Left = (p->Width - 230) / 2;
			this->lblPageInfo->Left = this->btnPrevPage->Right + 5;
			this->btnNextPage->Left = this->lblPageInfo->Right + 5;
		}

		System::Void CenterLoginCard(Object^ sender, LayoutEventArgs^ e)
		{
			if (this->panelLogin->Controls->Count >= 2) {
				Control^ loginCard = this->panelLogin->Controls[this->panelLogin->Controls->Count - 1];
				loginCard->Left = (this->panelLogin->Width - loginCard->Width) / 2;
				loginCard->Top = (this->panelLogin->Height - loginCard->Height) / 2 + 20;
			}
		}

		void Card_Click(Object^ sender, EventArgs^ e)
		{
			Control^ c = (Control^)sender;
			selectedMedicine = c->Tag->ToString();

			for each(DataRow ^ r in medicineTable->Rows) {
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString() == selectedMedicine) {
					this->txtName->Text = r[L"Name"]->ToString();
					this->numQty->Value = Convert::ToDecimal(r[L"Quantity"]);
					this->numPrice->Value = Convert::ToDecimal(r[L"Price"]);

					String^ cat = (r[L"Category"] != DBNull::Value) ? r[L"Category"]->ToString() : L"";
					for (int i = 0; i < this->cmbCategory->Items->Count; i++) {
						if (this->cmbCategory->Items[i]->ToString()->Contains(cat)) {
							this->cmbCategory->SelectedIndex = i;
							break;
						}
					}

					String^ age = (r[L"AgeGroup"] != DBNull::Value) ? r[L"AgeGroup"]->ToString() : L"Универсальный";
					bool ageFound = false;
					for (int i = 0; i < this->cmbAgeGroup->Items->Count; i++) {
						String^ itemText = this->cmbAgeGroup->Items[i]->ToString();
						String^ itemClean = itemText->Replace(L"👶 ", L"")->Replace(L"🧑 ", L"")
							->Replace(L"👴 ", L"")->Replace(L"🤰 ", L"")
							->Replace(L"🤱 ", L"")->Replace(L"👤 ", L"");
						if (itemText->Contains(age) || age->Contains(itemClean)) {
							this->cmbAgeGroup->SelectedIndex = i;
							ageFound = true;
							break;
						}
					}
					if (!ageFound) {
						for (int i = 0; i < this->cmbAgeGroup->Items->Count; i++) {
							if (this->cmbAgeGroup->Items[i]->ToString()->Contains(L"Универсальный")) {
								this->cmbAgeGroup->SelectedIndex = i;
								break;
							}
						}
					}

					String^ presc = (r[L"Prescription"] != DBNull::Value) ? r[L"Prescription"]->ToString() : L"Без рецепта";
					for (int i = 0; i < this->cmbPrescription->Items->Count; i++) {
						if (this->cmbPrescription->Items[i]->ToString()->Contains(presc)) {
							this->cmbPrescription->SelectedIndex = i;
							break;
						}
					}

					if (r[L"ExpiryDate"] != DBNull::Value)
						this->dtpExpiry->Value = Convert::ToDateTime(r[L"ExpiryDate"]);

					selectedImagePath = (r[L"ImagePath"] != DBNull::Value) ? r[L"ImagePath"]->ToString() : L"";
					if (!String::IsNullOrEmpty(selectedImagePath) && File::Exists(selectedImagePath))
						this->picMedicine->ImageLocation = selectedImagePath;
					else
						this->picMedicine->Image = nullptr;
					break;
				}
			}

			HighlightSelectedCard();
		}

		void HighlightSelectedCard()
		{
			DateTime today = DateTime::Now;
			DateTime warn30 = today.AddDays(30);

			for each(Control ^ ctrl in this->flowInventory->Controls) {
				Panel^ card = dynamic_cast<Panel^>(ctrl);
				if (card != nullptr && card->Tag != nullptr) {
					String^ name = card->Tag->ToString();
					bool isExpired = false;
					bool isSoon = false;

					for each(DataRow ^ row in medicineTable->Rows) {
						if (row->RowState != DataRowState::Deleted && row[L"Name"]->ToString() == name) {
							DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);
							isExpired = expiry < today;
							isSoon = !isExpired && expiry < warn30;
							break;
						}
					}

					if (name == selectedMedicine) {
						card->BackColor = Color::FromArgb(227, 242, 253);
					}
					else if (isExpired) {
						card->BackColor = Color::FromArgb(255, 235, 238);
					}
					else if (isSoon) {
						card->BackColor = Color::FromArgb(255, 253, 231);
					}
					else {
						card->BackColor = Color::White;
					}

					for each(Control ^ innerCtrl in card->Controls) {
						CheckBox^ chk = dynamic_cast<CheckBox^>(innerCtrl);
						if (chk != nullptr) {
							chk->BackColor = card->BackColor;
						}
					}
				}
			}
		}

		System::Void ChkSelect_CheckedChanged(Object^ sender, EventArgs^ e)
		{
			CheckBox^ chk = (CheckBox^)sender;
			String^ name = chk->Tag->ToString();
			if (chk->Checked) {
				if (!selectedForDelete->Contains(name))
					selectedForDelete->Add(name);
			}
			else {
				selectedForDelete->Remove(name);
			}
		}

		System::Void btnDeleteSelected_Click(Object^ sender, EventArgs^ e)
		{
			if (selectedForDelete->Count == 0) {
				MessageBox::Show(L"Отметьте галочками препараты для удаления!", L"Удаление");
				return;
			}

			String^ namesList = L"";
			for (int i = 0; i < Math::Min(5, selectedForDelete->Count); i++) {
				namesList += L"• " + selectedForDelete[i] + L"\n";
			}
			if (selectedForDelete->Count > 5)
				namesList += L"... и ещё " + (selectedForDelete->Count - 5).ToString() + L"\n";

			if (MessageBox::Show(String::Format(L"Будет удалено: {0} препаратов\n\n{1}\nПродолжить?",
				selectedForDelete->Count, namesList),
				L"Подтверждение", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) ==
				System::Windows::Forms::DialogResult::Yes) {

				int deletedCount = 0;
				for each(String ^ name in selectedForDelete) {
					for each(DataRow ^ row in medicineTable->Rows) {
						if (row->RowState != DataRowState::Deleted &&
							row[L"Name"]->ToString() == name) {
							AddHistory(L"Удаление", name, 0, Decimal(0));
							row->Delete();
							deletedCount++;
							break;
						}
					}
				}
				selectedForDelete->Clear();
				selectedMedicine = L"";
				this->picMedicine->Image = nullptr;
				RefreshCards();
				MessageBox::Show(String::Format(L"Удалено: {0} препаратов", deletedCount), L"Готово");
			}
		}

		System::Void btnDeleteExpired_Click(Object^ sender, EventArgs^ e)
		{
			DateTime today = DateTime::Now;
			System::Collections::ArrayList^ expiredList = gcnew System::Collections::ArrayList();

			for each(DataRow ^ row in medicineTable->Rows) {
				if (row->RowState == DataRowState::Deleted) continue;
				DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);
				if (expiry < today) {
					expiredList->Add(row[L"Name"]->ToString());
				}
			}

			if (expiredList->Count == 0) {
				MessageBox::Show(L"Нет просроченных препаратов!", L"Проверка");
				return;
			}

			String^ expiredNames = L"";
			for (int i = 0; i < Math::Min(10, expiredList->Count); i++) {
				expiredNames += L"• " + expiredList[i]->ToString() + L"\n";
			}
			if (expiredList->Count > 10) {
				expiredNames += L"... и ещё " + (expiredList->Count - 10).ToString() + L"\n";
			}

			if (MessageBox::Show(String::Format(L"Найдено просроченных: {0}\n\n{1}\nУдалить их все?",
				expiredList->Count, expiredNames),
				L"Просроченные препараты", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) ==
				System::Windows::Forms::DialogResult::Yes) {

				int deletedCount = 0;
				for (int i = medicineTable->Rows->Count - 1; i >= 0; i--) {
					DataRow^ row = medicineTable->Rows[i];
					if (row->RowState == DataRowState::Deleted) continue;

					DateTime expiry = Convert::ToDateTime(row[L"ExpiryDate"]);
					if (expiry < today) {
						AddHistory(L"Удаление просрочки", row[L"Name"]->ToString(), 0, Decimal(0));
						row->Delete();
						deletedCount++;
					}
				}

				selectedMedicine = L"";
				this->picMedicine->Image = nullptr;
				RefreshCards();
				MessageBox::Show(String::Format(L"Удалено просроченных: {0}", deletedCount), L"Готово");
			}
		}

		System::Void btnAddToCart_Click(Object^ sender, EventArgs^ e)
		{
			if (selectedMedicine == L"") {
				MessageBox::Show(L"Выберите препарат!", L"Внимание", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			int qty = Decimal::ToInt32(this->numQty->Value);
			if (qty <= 0) {
				MessageBox::Show(L"Укажите количество!", L"Внимание", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			for each(DataRow ^ r in cartTable->Rows) {
				if (r[L"Name"]->ToString() == selectedMedicine) {
					int newQty = Convert::ToInt32(r[L"Quantity"]) + qty;
					Decimal price = Convert::ToDecimal(r[L"Price"]);
					r[L"Quantity"] = newQty;
					r[L"Итого"] = Decimal::Multiply(price, Decimal(newQty));
					UpdateCartButton();
					return;
				}
			}
			Decimal priceVal = Decimal(0);
			for each(DataRow ^ r in medicineTable->Rows) {
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString() == selectedMedicine) {
					priceVal = Convert::ToDecimal(r[L"Price"]);
					break;
				}
			}
			DataRow^ nr = cartTable->NewRow();
			nr[L"Name"] = selectedMedicine;
			nr[L"Quantity"] = qty;
			nr[L"Price"] = priceVal;
			nr[L"Итого"] = Decimal::Multiply(priceVal, Decimal(qty));
			cartTable->Rows->Add(nr);
			AddHistory(L"В корзину", selectedMedicine, qty, Decimal::Multiply(priceVal, Decimal(qty)));
			UpdateCartButton();
		}

		System::Void btnShowCart_Click(Object^ sender, EventArgs^ e)
		{
			if (cartTable->Rows->Count == 0) {
				MessageBox::Show(L"Корзина пуста!", L"Корзина", MessageBoxButtons::OK, MessageBoxIcon::Information);
				return;
			}
			CartForm^ cf = gcnew CartForm(cartTable, medicineTable);
			cf->ShowDialog(this);
			if (cf->orderPlaced) {
				for each(DataRow ^ cr in cartTable->Rows) {
					AddHistory(L"Продажа", cr[L"Name"]->ToString(),
						Convert::ToInt32(cr[L"Quantity"]), Convert::ToDecimal(cr[L"Итого"]));
				}
			}
			RefreshCards();
		}

		System::Void btnSell_Click(Object^ sender, EventArgs^ e)
		{
			if (selectedMedicine == L"") {
				MessageBox::Show(L"Выберите препарат!", L"Внимание", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			int s = Decimal::ToInt32(this->numQty->Value);
			if (s <= 0) {
				MessageBox::Show(L"Укажите количество!", L"Внимание", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			for each(DataRow ^ r in medicineTable->Rows) {
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString() == selectedMedicine) {
					int cur = Convert::ToInt32(r[L"Quantity"]);
					Decimal price = Convert::ToDecimal(r[L"Price"]);
					if (s > cur) {
						MessageBox::Show(L"Недостаточно!\nДоступно: " + cur + L" шт.", L"Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
						return;
					}
					r[L"Quantity"] = cur - s;
					Decimal итог = Decimal::Multiply(price, Decimal(s));
					AddHistory(L"Продажа", selectedMedicine, s, итог);
					MessageBox::Show(String::Format(L"Чек\nПрепарат: {0}\nКол-во: {1} шт.\nИтого: {2:F2} сум", selectedMedicine, s, итог), L"Чек");
					break;
				}
			}
			RefreshCards();
		}

		System::Void btnRestock_Click(Object^ sender, EventArgs^ e)
		{
			if (selectedMedicine == L"") return;
			int a = Decimal::ToInt32(this->numQty->Value);
			if (a <= 0) return;
			for each(DataRow ^ r in medicineTable->Rows) {
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString() == selectedMedicine) {
					r[L"Quantity"] = Convert::ToInt32(r[L"Quantity"]) + a;
					AddHistory(L"Пополнение", selectedMedicine, a, Decimal(0));
					MessageBox::Show(String::Format(L"Пополнено: {0} + {1} шт.", selectedMedicine, a), L"Пополнение");
					break;
				}
			}
			RefreshCards();
		}

		System::Void btnAdd_Click(Object^ sender, EventArgs^ e)
		{
			String^ n = this->txtName->Text->Trim();
			if (n->Length == 0) {
				MessageBox::Show(L"Введите название препарата.", L"Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			String^ catRaw = this->cmbCategory->SelectedItem->ToString();
			int spacePos = catRaw->IndexOf(L" ");
			String^ cat = spacePos > 0 ? catRaw->Substring(spacePos + 1) : catRaw;

			String^ ageRaw = this->cmbAgeGroup->SelectedItem->ToString();
			spacePos = ageRaw->IndexOf(L" ");
			String^ age = spacePos > 0 ? ageRaw->Substring(spacePos + 1) : ageRaw;

			String^ prescRaw = this->cmbPrescription->SelectedItem->ToString();
			spacePos = prescRaw->IndexOf(L" ");
			String^ presc = spacePos > 0 ? prescRaw->Substring(spacePos + 1) : prescRaw;

			DataRow^ found = nullptr;
			for each(DataRow ^ r in medicineTable->Rows)
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString()->ToLower() == n->ToLower()) {
					found = r; break;
				}

			if (found != nullptr) {
				found[L"Quantity"] = Decimal::ToInt32(this->numQty->Value);
				found[L"Price"] = this->numPrice->Value;
				found[L"ImagePath"] = selectedImagePath;
				found[L"ExpiryDate"] = this->dtpExpiry->Value;
				found[L"Category"] = cat;
				found[L"AgeGroup"] = age;
				found[L"Prescription"] = presc;
				AddHistory(L"Обновление", n, Decimal::ToInt32(this->numQty->Value), this->numPrice->Value);
			}
			else {
				DataRow^ nr = medicineTable->NewRow();
				nr[L"Name"] = n;
				nr[L"Quantity"] = Decimal::ToInt32(this->numQty->Value);
				nr[L"Price"] = this->numPrice->Value;
				nr[L"ImagePath"] = selectedImagePath;
				nr[L"ExpiryDate"] = this->dtpExpiry->Value;
				nr[L"Category"] = cat;
				nr[L"AgeGroup"] = age;
				nr[L"Prescription"] = presc;
				medicineTable->Rows->Add(nr);
				AddHistory(L"Добавление", n, Decimal::ToInt32(this->numQty->Value), this->numPrice->Value);
			}
			this->txtName->Clear();
			this->numQty->Value = 0;
			this->numPrice->Value = 0;
			this->picMedicine->Image = nullptr;
			selectedImagePath = L"";
			RefreshCards();
		}

		System::Void btnSelectImage_Click(Object^ sender, EventArgs^ e)
		{
			OpenFileDialog^ ofd = gcnew OpenFileDialog();
			ofd->Filter = L"Изображения|*.jpg;*.jpeg;*.png;*.bmp";
			if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				String^ imagesFolder = Path::Combine(Application::StartupPath, L"Images");
				if (!Directory::Exists(imagesFolder)) Directory::CreateDirectory(imagesFolder);
				String^ fileName = Path::GetFileName(ofd->FileName);
				String^ newPath = Path::Combine(imagesFolder, fileName);
				try { File::Copy(ofd->FileName, newPath, true); selectedImagePath = newPath; }
				catch (...) { selectedImagePath = ofd->FileName; }
				this->picMedicine->ImageLocation = selectedImagePath;
			}
		}

		System::Void btnFindAnalogs_Click(Object^ sender, EventArgs^ e)
		{
			if (selectedMedicine == L"") {
				MessageBox::Show(L"Выберите препарат!", L"Поиск", MessageBoxButtons::OK, MessageBoxIcon::Information);
				return;
			}
			String^ searchName = selectedMedicine->ToLower();
			String^ selectedCategory = L"";
			for each(DataRow ^ r in medicineTable->Rows)
				if (r->RowState != DataRowState::Deleted && r[L"Name"]->ToString() == selectedMedicine) {
					selectedCategory = r[L"Category"]->ToString(); break;
				}

			DataTable^ analogTable = gcnew DataTable();
			analogTable->Columns->Add(L"Совпадение", String::typeid);
			analogTable->Columns->Add(L"Препарат", String::typeid);
			analogTable->Columns->Add(L"Категория", String::typeid);
			analogTable->Columns->Add(L"Цена", String::typeid);

			for each(DataRow ^ row in medicineTable->Rows) {
				if (row->RowState == DataRowState::Deleted) continue;
				String^ name = row[L"Name"]->ToString();
				if (name == selectedMedicine) continue;
				String^ cat = row[L"Category"]->ToString();
				String^ nameLower = name->ToLower();
				String^ matchType = L"";
				if (cat == selectedCategory) matchType = L"🔄 Та же категория";
				if (nameLower->Contains(searchName->Substring(0, Math::Min(4, searchName->Length)))) matchType = L"🔍 Похож";
				if (matchType != L"") {
					DataRow^ ar = analogTable->NewRow();
					ar[L"Совпадение"] = matchType;
					ar[L"Препарат"] = name;
					ar[L"Категория"] = cat;
					ar[L"Цена"] = Convert::ToDecimal(row[L"Price"]).ToString(L"F2") + L" сум";
					analogTable->Rows->Add(ar);
				}
			}
			if (analogTable->Rows->Count == 0) {
				MessageBox::Show(L"Похожие препараты не найдены", L"Поиск");
				return;
			}
			Form^ analogForm = gcnew Form();
			analogForm->Text = L"🔍 Похожие на: " + selectedMedicine;
			analogForm->Size = Drawing::Size(600, 400);
			analogForm->StartPosition = FormStartPosition::CenterParent;
			DataGridView^ dgvAnalogs = gcnew DataGridView();
			dgvAnalogs->DataSource = analogTable;
			dgvAnalogs->Dock = DockStyle::Fill;
			dgvAnalogs->ReadOnly = true;
			dgvAnalogs->AllowUserToAddRows = false;
			dgvAnalogs->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			analogForm->Controls->Add(dgvAnalogs);
			analogForm->ShowDialog(this);
		}

		System::Void btnHistory_Click(Object^ sender, EventArgs^ e)
		{
			Form^ historyForm = gcnew Form();
			historyForm->Text = L"📋 История операций";
			historyForm->Size = Drawing::Size(700, 450);
			historyForm->StartPosition = FormStartPosition::CenterParent;
			DataGridView^ dgvHistory = gcnew DataGridView();
			dgvHistory->DataSource = historyTable;
			dgvHistory->Dock = DockStyle::Fill;
			dgvHistory->ReadOnly = true;
			dgvHistory->AllowUserToAddRows = false;
			dgvHistory->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			historyForm->Controls->Add(dgvHistory);
			historyForm->ShowDialog(this);
		}

		System::Void btnExport_Click(Object^ sender, EventArgs^ e)
		{
			SaveFileDialog^ sfd = gcnew SaveFileDialog();
			sfd->Filter = L"CSV файл|*.csv";
			sfd->FileName = L"NurPharm_Склад_" + DateTime::Now.ToString(L"dd-MM-yyyy");
			if (sfd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				try {
					StreamWriter^ sw = gcnew StreamWriter(sfd->FileName, false, System::Text::Encoding::UTF8);
					sw->WriteLine(L"Название,Категория,Возраст,Рецепт,Количество,Цена,Срок");
					for each(DataRow ^ r in medicineTable->Rows) {
						if (r->RowState == DataRowState::Deleted) continue;
						// Убираем эмодзи из возраста для CSV
						String^ ageForCsv = r[L"AgeGroup"]->ToString();
						ageForCsv = ageForCsv->Replace(L"👶 ", L"")->Replace(L"🧑 ", L"")
							->Replace(L"👴 ", L"")->Replace(L"🤰 ", L"")
							->Replace(L"🤱 ", L"")->Replace(L"👤 ", L"");
						sw->WriteLine(String::Format(L"{0},{1},{2},{3},{4},{5:F2},{6}",
							r[L"Name"], r[L"Category"], ageForCsv, r[L"Prescription"],
							r[L"Quantity"], r[L"Price"],
							Convert::ToDateTime(r[L"ExpiryDate"]).ToString(L"dd.MM.yyyy")));
					}
					sw->Close();
					MessageBox::Show(L"Файл сохранён!", L"Экспорт");
				}
				catch (Exception^ ex) { MessageBox::Show(L"Ошибка: " + ex->Message, L"Ошибка"); }
			}
		}

		System::Void btnImport_Click(Object^ sender, EventArgs^ e)
		{
			OpenFileDialog^ ofd = gcnew OpenFileDialog();
			ofd->Filter = L"CSV файлы|*.csv|Все файлы|*.*";
			ofd->Title = L"Импорт из CSV";

			if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				try {
					StreamReader^ sr = nullptr;

					try {
						sr = gcnew StreamReader(ofd->FileName, System::Text::Encoding::UTF8);
					}
					catch (...) {
						try {
							sr = gcnew StreamReader(ofd->FileName, System::Text::Encoding::GetEncoding(1251));
						}
						catch (...) {
							sr = gcnew StreamReader(ofd->FileName, System::Text::Encoding::Default);
						}
					}

					String^ headerLine = sr->ReadLine();
					if (String::IsNullOrWhiteSpace(headerLine)) {
						sr->Close();
						MessageBox::Show(L"Файл пуст!", L"Ошибка");
						return;
					}

					wchar_t separator = ',';
					if (headerLine->Contains(L";") && !headerLine->Contains(L",")) {
						separator = ';';
					}

					int imported = 0;
					int errors = 0;
					String^ errorMessages = L"";

					while (!sr->EndOfStream) {
						String^ line = sr->ReadLine();
						if (String::IsNullOrWhiteSpace(line)) continue;

						try {
							array<String^>^ parts = line->Split(separator);

							if (parts->Length < 7) {
								errors++;
								continue;
							}

							for (int i = 0; i < parts->Length; i++) {
								parts[i] = parts[i]->Trim()->Replace(L"\"", L"");
							}

							String^ name = parts[0];
							String^ category = parts[1];
							String^ ageGroup = parts[2];
							// Нормализация возраста — добавляем эмодзи если их нет
							if (!ageGroup->Contains(L"👶") && !ageGroup->Contains(L"🧑") &&
								!ageGroup->Contains(L"👴") && !ageGroup->Contains(L"🤰") &&
								!ageGroup->Contains(L"🤱") && !ageGroup->Contains(L"👤")) {
								if (ageGroup->Contains(L"Детский")) ageGroup = L"👶 " + ageGroup;
								else if (ageGroup->Contains(L"Взрослый")) ageGroup = L"🧑 " + ageGroup;
								else if (ageGroup->Contains(L"60+")) ageGroup = L"👴 " + ageGroup;
								else if (ageGroup->Contains(L"Беременные")) ageGroup = L"🤰 " + ageGroup;
								else if (ageGroup->Contains(L"Кормящие")) ageGroup = L"🤱 " + ageGroup;
								else if (ageGroup->Contains(L"Универсальный")) ageGroup = L"👤 " + ageGroup;
							}
							String^ prescription = parts[3];

							if (String::IsNullOrWhiteSpace(name)) {
								errors++;
								continue;
							}

							bool exists = false;
							for each(DataRow ^ row in medicineTable->Rows) {
								if (row->RowState != DataRowState::Deleted &&
									row[L"Name"]->ToString()->ToLower() == name->ToLower()) {
									exists = true;
									break;
								}
							}

							if (exists) {
								errorMessages += L"Пропущен (существует): " + name + L"\n";
								errors++;
								continue;
							}

							int quantity = 0;
							try {
								quantity = Convert::ToInt32(parts[4]);
							}
							catch (...) {
								errorMessages += L"Ошибка количества: " + name + L"\n";
								errors++;
								continue;
							}

							Decimal price = Decimal(0);
							try {
								String^ priceStr = parts[5]->Replace(L".", L",");
								price = Convert::ToDecimal(priceStr);
							}
							catch (...) {
								try {
									price = Convert::ToDecimal(parts[5],
										System::Globalization::CultureInfo::GetCultureInfo("en-US"));
								}
								catch (...) {
									errorMessages += L"Ошибка цены: " + name + L"\n";
									errors++;
									continue;
								}
							}

							DateTime expiryDate;
							try {
								String^ dateStr = parts[6]->Trim();

								array<String^>^ dateFormats = gcnew array<String^>{
									L"dd.MM.yyyy",
										L"dd/MM/yyyy",
										L"dd-MM-yyyy",
										L"MM/dd/yyyy",
										L"yyyy-MM-dd",
										L"yyyy.MM.dd"
								};

								bool dateParsed = false;
								for each(String ^ format in dateFormats) {
									if (DateTime::TryParseExact(dateStr, format,
										System::Globalization::CultureInfo::InvariantCulture,
										System::Globalization::DateTimeStyles::None, expiryDate)) {
										dateParsed = true;
										break;
									}
								}

								if (!dateParsed) {
									expiryDate = Convert::ToDateTime(dateStr);
								}
							}
							catch (...) {
								errorMessages += L"Ошибка даты: " + name + L"\n";
								errors++;
								continue;
							}

							DataRow^ nr = medicineTable->NewRow();
							nr[L"Name"] = name;
							nr[L"Category"] = category;
							nr[L"AgeGroup"] = ageGroup;
							nr[L"Prescription"] = prescription;
							nr[L"Quantity"] = quantity;
							nr[L"Price"] = price;
							nr[L"ExpiryDate"] = expiryDate;
							nr[L"ImagePath"] = L"";

							medicineTable->Rows->Add(nr);
							AddHistory(L"Импорт", name, quantity, price);
							imported++;
						}
						catch (Exception^ ex) {
							errors++;
							errorMessages += L"Ошибка в строке: " + ex->Message + L"\n";
						}
					}

					sr->Close();

					String^ resultMessage = String::Format(
						L"Импорт завершен!\n\n✅ Успешно: {0}\n❌ Ошибок: {1}",
						imported, errors);

					if (errorMessages->Length > 0 && errorMessages->Length < 500) {
						resultMessage += L"\n\nОшибки:\n" + errorMessages;
					}
					else if (errorMessages->Length >= 500) {
						resultMessage += L"\n\nСлишком много ошибок для отображения.";
					}

					MessageBox::Show(resultMessage, L"Результат импорта",
						MessageBoxButtons::OK,
						errors > 0 ? MessageBoxIcon::Warning : MessageBoxIcon::Information);

					if (imported > 0) {
						RefreshCards();
					}
				}
				catch (Exception^ ex) {
					MessageBox::Show(L"Критическая ошибка импорта: " + ex->Message +
						L"\n\nПроверьте формат файла.\nФайл должен быть в кодировке UTF-8 или Windows-1251.",
						L"Ошибка");
				}
			}
		}

		System::Void btnDoLogin_Click(Object^ sender, EventArgs^ e)
		{
			String^ u = this->txtUser->Text->Trim();
			String^ p = this->txtPass->Text;
			if (u->Length == 0) {
				MessageBox::Show(L"Введите имя пользователя.", L"Вход");
				return;
			}
			for each(DataRow ^ r in usersTable->Rows) {
				if (r[L"Username"]->ToString() == u && r[L"Password"]->ToString() == p) {
					this->panelLogin->Visible = false;
					this->panelMain->Visible = true;
					RefreshCards();
					CheckLowStock();
					return;
				}
			}
			MessageBox::Show(L"Неверное имя пользователя или пароль!", L"Ошибка входа");
		}

		System::Void btnDoRegister_Click(Object^ sender, EventArgs^ e)
		{
			String^ u = this->txtUser->Text->Trim();
			String^ p = this->txtPass->Text->Trim();
			if (u->Length < 3) { MessageBox::Show(L"Логин не менее 3 символов!"); return; }
			if (p->Length < 4) { MessageBox::Show(L"Пароль не менее 4 символов!"); return; }
			for each(DataRow ^ r in usersTable->Rows)
				if (r[L"Username"]->ToString()->ToLower() == u->ToLower()) {
					MessageBox::Show(L"Пользователь уже существует!"); return;
				}
			DataRow^ nr = usersTable->NewRow();
			nr[L"Username"] = u;
			nr[L"Password"] = p;
			usersTable->Rows->Add(nr);
			MessageBox::Show(L"Регистрация прошла успешно!", L"Готово");
		}

		System::Void btnLogout_Click(Object^ sender, EventArgs^ e)
		{
			cartTable->Rows->Clear();
			this->panelMain->Visible = false;
			this->panelLogin->Visible = true;
			this->txtPass->Clear();
			selectedMedicine = L"";
			selectedForDelete->Clear();
		}

		System::Void btnPrevPage_Click(Object^ sender, EventArgs^ e) { if (currentPage > 0) { currentPage--; RefreshCards(); } }
		System::Void btnNextPage_Click(Object^ sender, EventArgs^ e) { currentPage++; RefreshCards(); }
		System::Void txtSearch_TextChanged(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }
		System::Void cmbCatFilter_Changed(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }
		System::Void cmbExpiryFilter_Changed(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }
		System::Void cmbAgeFilter_Changed(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }
		System::Void cmbPrescriptionFilter_Changed(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }
		System::Void cmbSortBy_Changed(Object^ sender, EventArgs^ e) { currentPage = 0; RefreshCards(); }

		System::Void MyForm_Load(Object^ sender, EventArgs^ e)
		{
			if (File::Exists(L"inventory.xml")) {
				try { medicineTable->ReadXml(L"inventory.xml"); }
				catch (...) {}
			}
			if (File::Exists(L"history.xml")) {
				try { historyTable->ReadXml(L"history.xml"); }
				catch (...) {}
			}

			if (medicineTable->Rows->Count == 0) {
				AddMedicineHelper(L"Аспирин 500 мг", 100, 2500.0, L"Таблетки", L"👴 60+ (осторожно)", L"Без рецепта", 2027, 6, 1);
				AddMedicineHelper(L"Детский Панадол", 50, 4500.0, L"Сиропы", L"👶 Детский (0-12)", L"Без рецепта", 2027, 8, 15);
				AddMedicineHelper(L"Парацетамол 500 мг", 80, 3500.0, L"Таблетки", L"🤰 Беременные", L"Без рецепта", 2027, 3, 15);
				AddMedicineHelper(L"Амоксициллин 500 мг", 25, 18000.0, L"Таблетки", L"🧑 Взрослый (12-60)", L"По рецепту", 2027, 8, 10);
				AddMedicineHelper(L"Цефтриаксон 1г", 20, 25000.0, L"Ампулы", L"🧑 Взрослый (12-60)", L"По рецепту", 2027, 11, 30);
				AddMedicineHelper(L"Валидол", 35, 7500.0, L"Таблетки", L"👴 60+ (осторожно)", L"Без рецепта", 2026, 9, 30);
				AddMedicineHelper(L"Витамин С 500 мг", 120, 3000.0, L"Витамины", L"👤 Универсальный", L"Без рецепта", 2028, 6, 15);
				AddMedicineHelper(L"Омега-3 комплекс", 60, 15000.0, L"БАДы", L"🤱 Кормящие", L"Без рецепта", 2028, 6, 15);
				AddMedicineHelper(L"Оциллококцинум", 15, 28000.0, L"Гомеопатия", L"👤 Универсальный", L"Без рецепта", 2028, 3, 20);
				AddMedicineHelper(L"Шприц 5мл", 200, 1500.0, L"Шприцы", L"👤 Универсальный", L"Без рецепта", 2030, 1, 1);
				AddMedicineHelper(L"Система для капельницы", 30, 8500.0, L"Капельницы", L"🧑 Взрослый (12-60)", L"По рецепту", 2029, 6, 1);
				AddMedicineHelper(L"Бинт стерильный", 40, 6000.0, L"Перевязочные", L"👤 Универсальный", L"Без рецепта", 2029, 1, 1);
				AddMedicineHelper(L"Йод 5%", 50, 4500.0, L"Растворы", L"👤 Универсальный", L"Без рецепта", 2026, 7, 15);
				AddMedicineHelper(L"Нурофен суспензия", 20, 35000.0, L"Сиропы", L"👶 Детский (0-12)", L"Без рецепта", 2027, 12, 1);
				AddMedicineHelper(L"Левомеколь мазь", 55, 6500.0, L"Мази", L"👤 Универсальный", L"Без рецепта", 2028, 5, 10);
			}

			if (File::Exists(L"users.xml")) {
				try { usersTable->ReadXml(L"users.xml"); }
				catch (...) {}
			}
			if (usersTable->Rows->Count == 0) {
				DataRow^ r = usersTable->NewRow();
				r[L"Username"] = L"admin"; r[L"Password"] = L"admin";
				usersTable->Rows->Add(r);
			}

			currentPage = 0;
			RefreshCards();
		}

		System::Void MyForm_FormClosing(Object^ sender, FormClosingEventArgs^ e)
		{
			try {
				medicineTable->WriteXml(L"inventory.xml");
				usersTable->WriteXml(L"users.xml");
				historyTable->WriteXml(L"history.xml");
			}
			catch (...) {}
		}
	};
} 