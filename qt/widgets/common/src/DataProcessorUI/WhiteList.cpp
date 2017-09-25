#include "MantidQtWidgets/Common/DataProcessorUI/WhiteList.h"

#include <QString>

namespace MantidQt {
namespace MantidWidgets {
namespace DataProcessor {

Column::Column(QString const &name, QString const &algorithmProperty,
               bool isShown, QString const &prefix, QString const &description)
    : m_name(name), m_algorithmProperty(algorithmProperty), m_isShown(isShown),
      m_prefix(prefix), m_description(description) {}

QString const &Column::algorithmProperty() const { return m_algorithmProperty; }

bool Column::isShown() const { return m_isShown; }

QString const &Column::prefix() const { return m_prefix; }

QString const &Column::description() const { return m_description; }

QString const &Column::name() const { return m_name; }

ConstColumnIterator::ConstColumnIterator(QStringIterator names,
                                         QStringIterator descriptions,
                                         QStringIterator algorithmProperties,
                                         BoolIterator isShown,
                                         QStringIterator prefixes)
    : m_names(names), m_descriptions(descriptions),
      m_algorithmProperties(algorithmProperties), m_isShown(isShown),
      m_prefixes(prefixes) {}

ConstColumnIterator &ConstColumnIterator::operator++() {
  ++m_names;
  ++m_descriptions;
  ++m_algorithmProperties;
  ++m_isShown;
  ++m_prefixes;
  return (*this);
}

ConstColumnIterator ConstColumnIterator::operator++(int) {
  auto result = (*this);
  ++result;
  return result;
}

bool ConstColumnIterator::operator==(const ConstColumnIterator &other) const {
  return m_names == other.m_names;
}

bool ConstColumnIterator::operator!=(const ConstColumnIterator &other) const {
  return !((*this) == other);
}

auto ConstColumnIterator::operator*() const -> reference {
  return reference(*m_names, *m_algorithmProperties, *m_isShown, *m_prefixes,
                   *m_descriptions);
}

ConstColumnIterator &ConstColumnIterator::operator+=(difference_type n) {
  m_names += n;
  m_algorithmProperties += n;
  m_isShown += n;
  m_prefixes += n;
  m_descriptions += n;
  return (*this);
}

ConstColumnIterator &ConstColumnIterator::operator-=(difference_type n) {
  m_names -= n;
  m_algorithmProperties -= n;
  m_isShown -= n;
  m_prefixes -= n;
  m_descriptions -= n;
  return (*this);
}

/** Adds an element to the whitelist
* @param colName : the name of the column to be added
* @param algProperty : the name of the property linked to this column
* @param showValue : true if we want to use what's in this column to
* generate
* the output ws name
* @param prefix : the prefix to be added to the value of this column
* @param description : a description of this column
*/
void WhiteList::addElement(const QString &colName, const QString &algProperty,
                           const QString &description, bool isShown,
                           const QString &prefix) {
  m_names.emplace_back(colName);
  m_algorithmProperties.emplace_back(algProperty);
  m_isShown.emplace_back(isShown);
  m_prefixes.emplace_back(prefix);
  m_descriptions.emplace_back(description);
}

/** Returns the column index for a column specified via its name
    @param colName : The column name
*/
int WhiteList::indexFromName(const QString &columnName) const {
  auto nameIt = std::find(m_names.cbegin(), m_names.cend(), columnName);
  return static_cast<int>(std::distance(m_names.cbegin(), nameIt));
}

/** Returns the column name for a column specified via its index
    @param index : The column index
*/
QString WhiteList::name(int index) const { return m_names.at(index); }

/** Returns the algorithm property linked to a column specified via its index
    @param index : The column index
*/
QString WhiteList::algorithmProperty(int index) const {
  return m_algorithmProperties.at(index);
}

/** Returns the column description for a column specified via its index
    @param index : The column index
*/
QString WhiteList::description(int index) const {
  return m_descriptions.at(index);
}

/** Returns the size of this whitelist, i.e. the number of columns
*/
size_t WhiteList::size() const { return m_names.size(); }

/** Returns true if the contents of this column should be used to generate the
 * name of the output ws
 * @param index : The column index
*/
bool WhiteList::isShown(int index) const { return m_isShown.at(index); }

/** Returns the column prefix used to generate the name of the output ws (will
* only be used if showValue is true for this column
* @param index : The column index
*/
QString WhiteList::prefix(int index) const { return m_prefixes.at(index); }

std::vector<QString> const &WhiteList::names() const { return m_names; }

auto WhiteList::end() const -> const_iterator { return cend(); }

auto WhiteList::begin() const -> const_iterator { return cbegin(); }

auto WhiteList::cbegin() const -> const_iterator {
  return const_iterator(m_names.cbegin(), m_descriptions.cbegin(),
                        m_algorithmProperties.cbegin(), m_isShown.cbegin(),
                        m_prefixes.cbegin());
}

auto WhiteList::cend() const -> const_iterator {
  return const_iterator(m_names.cend(), m_descriptions.cend(),
                        m_algorithmProperties.cend(), m_isShown.cend(),
                        m_prefixes.cend());
}

ConstColumnIterator operator+(const ConstColumnIterator &lhs,
                              typename ConstColumnIterator::difference_type n) {
  auto result = lhs;
  result += n;
  return result;
}

ConstColumnIterator operator+(typename ConstColumnIterator::difference_type n,
                              const ConstColumnIterator &rhs) {
  auto result = rhs;
  result += n;
  return result;
}

ConstColumnIterator operator-(const ConstColumnIterator &lhs,
                              typename ConstColumnIterator::difference_type n) {
  auto result = lhs;
  result -= n;
  return result;

}

ConstColumnIterator operator-(typename ConstColumnIterator::difference_type n,
                              const ConstColumnIterator &rhs) {
  auto result = rhs;
  result -= n;
  return result;
}
}
}
}