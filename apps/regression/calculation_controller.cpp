#include "calculation_controller.h"
#include "../apps_container.h"
#include "../shared/poincare_helpers.h"
#include <poincare/char_layout.h>
#include <poincare/vertical_offset_layout.h>

#include <assert.h>

using namespace Poincare;
using namespace Shared;

namespace Regression {

static inline int max(int x, int y) { return (x>y ? x : y); }

CalculationController::CalculationController(Responder * parentResponder, ButtonRowController * header, Store * store) :
  TabTableController(parentResponder),
  ButtonRowDelegate(header, nullptr),
  m_selectableTableView(this, this, this, this),
  m_titleCells{},
  m_r2TitleCell(),
  m_columnTitleCells{},
  m_doubleCalculationCells{},
  m_calculationCells{},
  m_hideableCell(),
  m_store(store)
{
  m_r2Layout = HorizontalLayout::Builder(CharLayout::Builder('r', KDFont::SmallFont), VerticalOffsetLayout::Builder(CharLayout::Builder('2', KDFont::SmallFont), VerticalOffsetLayoutNode::Type::Superscript));
  m_selectableTableView.setVerticalCellOverlap(0);
  m_selectableTableView.setBackgroundColor(Palette::WallScreenDark);
  m_selectableTableView.setMargins(k_margin, k_scrollBarMargin, k_scrollBarMargin, k_margin);
  m_r2TitleCell.setAlignment(1.0f, 0.5f);
  for (int i = 0; i < Store::k_numberOfSeries; i++) {
    m_columnTitleCells[i].setParentResponder(&m_selectableTableView);
  }
  for (int i = 0; i < k_numberOfDoubleCalculationCells; i++) {
    m_doubleCalculationCells[i].setTextColor(Palette::GreyDark);
    m_doubleCalculationCells[i].setParentResponder(&m_selectableTableView);
  }
  for (int i = 0; i < k_numberOfCalculationCells;i++) {
    m_calculationCells[i].setTextColor(Palette::GreyDark);
  }
  m_hideableCell.setHide(true);
}

const char * CalculationController::title() {
  return I18n::translate(I18n::Message::StatTab);
}

bool CalculationController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Up) {
    selectableTableView()->deselectTable();
    app()->setFirstResponder(tabController());
    return true;
  }
  return false;
}

void CalculationController::didBecomeFirstResponder() {
  if (selectedRow() == -1) {
    selectCellAtLocation(1, 0);
  } else {
    selectCellAtLocation(selectedColumn(), selectedRow());
  }
  TabTableController::didBecomeFirstResponder();
}

void CalculationController::tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY) {
  /* To prevent selecting cell with no content (top left corner of the table),
   * as soon as the selected cell is the top left corner, we either reselect
   * the previous cell or select the tab controller depending on from which cell
   * the selection comes. This trick does not create an endless loop as the
   * previous cell cannot be the top left corner cell if it also is the
   * selected one. */
  if (t->selectedRow() == 0 && t->selectedColumn() == 0) {
    if (previousSelectedCellX == 0 && previousSelectedCellY == 1) {
      selectableTableView()->deselectTable();
      app()->setFirstResponder(tabController());
    } else {
      t->selectCellAtLocation(0, 1);
    }
  }
  if (t->selectedColumn() > 0 && t->selectedRow() >= 0 && t->selectedRow() <= k_totalNumberOfDoubleBufferRows) {
    // If we are on a double text cell, we have to choose which subcell to select
    EvenOddDoubleBufferTextCellWithSeparator * myCell = (EvenOddDoubleBufferTextCellWithSeparator *)t->selectedCell();
    // Default selected subcell is the left one
    bool firstSubCellSelected = true;
    if (previousSelectedCellX > 0 && previousSelectedCellY >= 0 && previousSelectedCellY <= k_totalNumberOfDoubleBufferRows) {
      // If we come from another double text cell, we have to update subselection
      EvenOddDoubleBufferTextCellWithSeparator * myPreviousCell = (EvenOddDoubleBufferTextCellWithSeparator *)t->cellAtLocation(previousSelectedCellX, previousSelectedCellY);
      /* If the selection stays in the same column, we copy the subselection
       * from previous cell. Otherwise, the selection has jumped to another
       * column, we thus subselect the other subcell. */
       assert(myPreviousCell);
      firstSubCellSelected = t->selectedColumn() == previousSelectedCellX ? myPreviousCell->firstTextSelected() : !myPreviousCell->firstTextSelected();
    }
    myCell->selectFirstText(firstSubCellSelected);
  }
}

bool CalculationController::isEmpty() const {
  return m_store->isEmpty();
}

I18n::Message CalculationController::emptyMessage() {
  return I18n::Message::NoValueToCompute;
}

Responder * CalculationController::defaultController() {
  return tabController();
}

int CalculationController::numberOfRows() {
  return 1 + k_totalNumberOfDoubleBufferRows + 4 + maxNumberOfCoefficients() + hasLinearRegression() * 2;
}

int CalculationController::numberOfColumns() {
  return 1 + m_store->numberOfNonEmptySeries();
}

void CalculationController::willDisplayCellAtLocation(HighlightCell * cell, int i, int j) {
  if (i == 0 && j == 0) {
    return;
  }
  EvenOddCell * myCell = (EvenOddCell *)cell;
  myCell->setEven(j%2 == 0);
  myCell->setHighlighted(i == selectedColumn() && j == selectedRow());

  // Calculation title
  if (i == 0) {
    bool shouldDisplayRAndR2 = hasLinearRegression();
    int numberRows = numberOfRows();
    if (shouldDisplayRAndR2 && j == numberRows-1) {
      EvenOddExpressionCell * myCell = (EvenOddExpressionCell *)cell;
      myCell->setLayout(m_r2Layout);
      return;
    }
    MarginEvenOddMessageTextCell * myCell = (MarginEvenOddMessageTextCell *)cell;
    myCell->setAlignment(1.0f, 0.5f);
    if (j <= k_regressionCellIndex) {
      I18n::Message titles[k_regressionCellIndex] = {I18n::Message::Mean, I18n::Message::Sum, I18n::Message::SquareSum, I18n::Message::StandardDeviation, I18n::Message::Deviation, I18n::Message::NumberOfDots, I18n::Message::Covariance, I18n::Message::Sxy, I18n::Message::Regression};
      myCell->setMessage(titles[j-1]);
      return;
    }
    if (shouldDisplayRAndR2 && j == numberRows - 2) {
      myCell->setMessage(I18n::Message::R);
      return;
    }
    I18n::Message titles[5] = {I18n::Message::A, I18n::Message::B, I18n::Message::C, I18n::Message::D, I18n::Message::E};
    myCell->setMessage(titles[j - k_regressionCellIndex - 1]);
    return;
  }

  int seriesNumber = m_store->indexOfKthNonEmptySeries(i - 1);
  assert(i >= 0 && seriesNumber < DoublePairStore::k_numberOfSeries);

  // Coordinate and series title
  if (j == 0 && i > 0) {
    ColumnTitleCell * myCell = (ColumnTitleCell *)cell;
    char buffer[] = {'X', static_cast<char>('1' + seriesNumber), 0};
    myCell->setFirstText(buffer);
    buffer[0] = 'Y';
    myCell->setSecondText(buffer);
    myCell->setColor(Palette::DataColor[seriesNumber]);
    return;
  }

  // Calculation cell
  if (i > 0 && j > 0 && j <= k_totalNumberOfDoubleBufferRows) {
    ArgCalculPointer calculationMethods[k_totalNumberOfDoubleBufferRows] = {&Store::meanOfColumn, &Store::sumOfColumn, &Store::squaredValueSumOfColumn, &Store::standardDeviationOfColumn, &Store::varianceOfColumn};
    double calculation1 = (m_store->*calculationMethods[j-1])(seriesNumber, 0);
    double calculation2 = (m_store->*calculationMethods[j-1])(seriesNumber, 1);
    EvenOddDoubleBufferTextCellWithSeparator * myCell = (EvenOddDoubleBufferTextCellWithSeparator *)cell;
    char buffer[PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
    PoincareHelpers::ConvertFloatToText<double>(calculation1, buffer, PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
    myCell->setFirstText(buffer);
    PoincareHelpers::ConvertFloatToText<double>(calculation2, buffer, PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
    myCell->setSecondText(buffer);
    return;
  }
  SeparatorEvenOddBufferTextCell * bufferCell = (SeparatorEvenOddBufferTextCell *)cell;
  if (i > 0 && j == k_regressionCellIndex) {
    Model * model = m_store->modelForSeries(seriesNumber);
    const char * formula = I18n::translate(model->formulaMessage());
    bufferCell->setText(formula);
    return;
  }
  if (i > 0 && j > k_totalNumberOfDoubleBufferRows && j < k_regressionCellIndex) {
    assert(j != k_regressionCellIndex);
    CalculPointer calculationMethods[] = {&Store::doubleCastedNumberOfPairsOfSeries, &Store::covariance, &Store::columnProductSum};
    double calculation = (m_store->*calculationMethods[j-k_totalNumberOfDoubleBufferRows-1])(seriesNumber);
    char buffer[PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
    PoincareHelpers::ConvertFloatToText<double>(calculation, buffer, PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
    bufferCell->setText(buffer);
    return;
  }
  if (i > 0 && j > k_totalNumberOfDoubleBufferRows) {
    assert(j > k_regressionCellIndex);
    int maxNumberCoefficients = maxNumberOfCoefficients();
    Model::Type modelType = m_store->seriesRegressionType(seriesNumber);

    // Put dashes if regression is not defined
    Poincare::Context * globContext = const_cast<AppsContainer *>(static_cast<const AppsContainer *>(app()->container()))->globalContext();
    double * coefficients = m_store->coefficientsForSeries(seriesNumber, globContext);
    bool coefficientsAreDefined = true;
    int numberOfCoefs = m_store->modelForSeries(seriesNumber)->numberOfCoefficients();
    for (int i = 0; i < numberOfCoefs; i++) {
      if (std::isnan(coefficients[i])) {
        coefficientsAreDefined = false;
        break;
      }
    }
    if (!coefficientsAreDefined) {
       bufferCell->setText(I18n::translate(I18n::Message::Dash));
       return;
    }

    if (j > k_regressionCellIndex + maxNumberCoefficients) {
      // Fill r and r2 if needed
      if (modelType == Model::Type::Linear) {
        CalculPointer calculationMethods[2] = {&Store::correlationCoefficient, &Store::squaredCorrelationCoefficient};
        double calculation = (m_store->*calculationMethods[j - k_regressionCellIndex - maxNumberCoefficients - 1])(seriesNumber);
        char buffer[PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
        PoincareHelpers::ConvertFloatToText<double>(calculation, buffer, PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
        bufferCell->setText(buffer);
        return;
      } else {
        bufferCell->setText(I18n::translate(I18n::Message::Dash));
        return;
      }
    } else {
      // Fill the current coefficient if needed
      int currentNumberOfCoefs = m_store->modelForSeries(seriesNumber)->numberOfCoefficients();
      if (j > k_regressionCellIndex + currentNumberOfCoefs) {
        bufferCell->setText(I18n::translate(I18n::Message::Dash));
        return;
      } else {
        char buffer[PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
        PoincareHelpers::ConvertFloatToText<double>(coefficients[j - k_regressionCellIndex - 1], buffer, PrintFloat::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits);
        bufferCell->setText(buffer);
        return;
      }
    }
  }
}

KDCoordinate CalculationController::columnWidth(int i) {
  if (i == 0) {
    return k_titleCalculationCellWidth;
  }
  Model::Type currentType = m_store->seriesRegressionType(m_store->indexOfKthNonEmptySeries(i-1));
  if (currentType == Model::Type::Quartic) {
    return k_quarticCalculationCellWidth;
  }
  if (currentType == Model::Type::Cubic) {
    return k_cubicCalculationCellWidth;
  }
  return k_minCalculationCellWidth;
}

KDCoordinate CalculationController::rowHeight(int j) {
  return k_cellHeight;
}

KDCoordinate CalculationController::cumulatedHeightFromIndex(int j) {
  return j*rowHeight(0);
}

int CalculationController::indexFromCumulatedHeight(KDCoordinate offsetY) {
  return (offsetY-1) / rowHeight(0);
}

HighlightCell * CalculationController::reusableCell(int index, int type) {
  if (type == k_standardCalculationTitleCellType) {
    assert(index >= 0 && index < k_maxNumberOfDisplayableRows);
    return &m_titleCells[index];
  }
  if (type == k_r2CellType) {
    assert(index == 0);
    return &m_r2TitleCell;
  }
  if (type == k_columnTitleCellType) {
    assert(index >= 0 && index < Store::k_numberOfSeries);
    return &m_columnTitleCells[index];
  }
  if (type == k_doubleBufferCalculationCellType) {
    assert(index >= 0 && index < k_numberOfDoubleCalculationCells);
    return &m_doubleCalculationCells[index];
  }
  if (type == k_hideableCellType) {
    return &m_hideableCell;
  }
  assert(index >= 0 && index < k_numberOfCalculationCells);
  return &m_calculationCells[index];
}

int CalculationController::reusableCellCount(int type) {
  if (type == k_standardCalculationTitleCellType) {
    return k_maxNumberOfDisplayableRows;
  }
  if (type == k_r2CellType) {
    return 1;
  }
  if (type == k_columnTitleCellType) {
    return Store::k_numberOfSeries;
  }
  if (type == k_doubleBufferCalculationCellType) {
    return k_numberOfDoubleCalculationCells;
  }
  if (type == k_hideableCellType) {
    return 1;
  }
  assert(type == k_standardCalculationCellType);
  return k_numberOfCalculationCells;
}

int CalculationController::typeAtLocation(int i, int j) {
  if (i == 0 && j == 0) {
    return k_hideableCellType;
  }
  bool shouldDisplayRAndR2 = hasLinearRegression();
  int numberRows = numberOfRows();
  if (shouldDisplayRAndR2 && i == 0 && j == numberRows-1) {
    return k_r2CellType;
  }
  if (i == 0) {
    return k_standardCalculationTitleCellType;
  }
  if (j == 0) {
    return k_columnTitleCellType;
  }
  if (j > 0 && j <= k_totalNumberOfDoubleBufferRows) {
    return k_doubleBufferCalculationCellType;
  }
  return k_standardCalculationCellType;
}

Responder * CalculationController::tabController() const {
  return (parentResponder()->parentResponder()->parentResponder());
}

bool CalculationController::hasLinearRegression() const {
  int numberOfDefinedSeries = m_store->numberOfNonEmptySeries();
  for (int i = 0; i < numberOfDefinedSeries; i++) {
    if (m_store->seriesRegressionType(m_store->indexOfKthNonEmptySeries(i)) == Model::Type::Linear) {
      return true;
    }
  }
  return false;
}

int CalculationController::maxNumberOfCoefficients() const {
  int maxNumberCoefficients = 0;
  int numberOfDefinedSeries = m_store->numberOfNonEmptySeries();
  for (int i = 0; i < numberOfDefinedSeries; i++) {
    int currentNumberOfCoefs = m_store->modelForSeries(m_store->indexOfKthNonEmptySeries(i))->numberOfCoefficients();
    maxNumberCoefficients = max(maxNumberCoefficients, currentNumberOfCoefs);
  }
  return maxNumberCoefficients;
}

}

