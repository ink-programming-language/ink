// Translated from solution.cpp.

var MAXN: dynamic = cpp_cast((4000));

var SIGNATURE: dynamic = ["b9", "91", "af", "fc", "fb", "24", "db", "04", "76", "fe", "95", "76", "b9", "03", "95", "2e"];

var MOD: dynamic = cpp_cast(((1e9 + 7)));

func nextInt() -> dynamic
{
  var d: dynamic = cpp_uninitialized();
  read(d);
  return d;
}

func nextString() -> dynamic
{
  var d: dynamic = cpp_uninitialized();
  read(d);
  return d;
}

func nextChar() -> dynamic
{
  var d: dynamic = cpp_uninitialized();
  read(d);
  return d;
}

func isPair(l: dynamic, r: dynamic) -> dynamic
{
  return (cpp_binary(((l == cpp_char("(")) && (r == cpp_char(")"))), "or", ((l == cpp_char("[")) && (r == cpp_char("]")))));
}

func slurp(filename: dynamic) -> dynamic
{
  var str: dynamic = cpp_uninitialized();
  (str << in_cpp.rdbuf());
  return str.str();
}

func split(hay: dynamic, delim: dynamic, delim2: dynamic = cpp_char("\u{0}")) -> dynamic
{
  var answer: dynamic = cpp_uninitialized();
  var buffer: dynamic = cpp_uninitialized();
  for (var chr: dynamic in hay)
  {
    if (((chr == delim) || (chr == delim2)))
    {
      answer.push_back(buffer);
      buffer = "";
    } else
    {
      buffer.push_back(chr);
    }
  }
  answer.push_back(buffer);
  return answer;
}

func isEven(number: dynamic) -> dynamic
{
  var last: dynamic = (((*number.rbegin())) - cpp_char("0"));
  return (((last % 2) == 0));
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = "";
  while (cpp_update(b, "--"))
  {
    res += a;
  }
  return res;
}

func isPali(a: dynamic) -> dynamic
{
  var n: dynamic = cpp_cast(a.size());
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(n)))
    {
      if ((a[i] != a[((n - i) - 1)]))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func countPairs(n: dynamic) -> dynamic
{
  return ((n * ((n + 1))) / 2);
}

func makeNumPair(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * 10000) + b);
}

func getStringOrInt() -> dynamic
{
  return ( ((rand() % 2)) ? "string" : 0);
}

class trio
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
}

func toLower(src: dynamic) -> dynamic
{
  var tmp: dynamic = src;
  transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
  return tmp;
}

func main() -> dynamic
{
  solve();
  return 0;
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_uninitialized();
  while (cpp_update(n, "--"))
  {
    var d: dynamic = cpp_uninitialized();
    read(d);
    a.push_back(d);
  }
  n = a.size();
  sort(a.begin(), a.end());
  assert((n % 2));
  write(a[(n / 2)], "\n");
}
