// Translated from solution.cpp.

var MAXN = cpp_cast((4000));

var SIGNATURE = ["b9", "91", "af", "fc", "fb", "24", "db", "04", "76", "fe", "95", "76", "b9", "03", "95", "2e"];

var MOD = cpp_cast(((1e9 + 7)));

func nextInt()
{
  var d: dynamic;
  read(d);
  return d;
}

func nextString()
{
  var d: dynamic;
  read(d);
  return d;
}

func nextChar()
{
  var d: dynamic;
  read(d);
  return d;
}

func isPair(l: dynamic, r: dynamic)
{
  return (cpp_binary(((l == cpp_char("(")) && (r == cpp_char(")"))), "or", ((l == cpp_char("[")) && (r == cpp_char("]")))));
}

func slurp(filename: dynamic)
{
  var str: dynamic;
  (str << in_cpp.rdbuf());
  return str.str();
}

func split(hay: dynamic, delim: dynamic, delim2: dynamic = cpp_char("\u{0}"))
{
  var answer: dynamic;
  var buffer: dynamic;
  for (var chr in hay)
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

func isEven(number: dynamic)
{
  var last = (((*number.rbegin())) - cpp_char("0"));
  return (((last % 2) == 0));
}

func operator_multiply(a: dynamic, b: dynamic)
{
  var res = "";
  while (cpp_update(b, "--"))
  {
    res += a;
  }
  return res;
}

func isPali(a: dynamic)
{
  var n = cpp_cast(a.size());
  {
    var i = 0;
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

func countPairs(n: dynamic)
{
  return ((n * ((n + 1))) / 2);
}

func makeNumPair(a: dynamic, b: dynamic)
{
  return ((a * 10000) + b);
}

func getStringOrInt()
{
  return (if ((rand() % 2)) "string" else 0);
}

class trio
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
}

func toLower(src: dynamic)
{
  var tmp = src;
  transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
  return tmp;
}

func main()
{
  solve();
  return 0;
}

func solve()
{
  var n: dynamic;
  read(n);
  var a: dynamic;
  while (cpp_update(n, "--"))
  {
    var d: dynamic;
    read(d);
    a.push_back(d);
  }
  n = a.size();
  sort(a.begin(), a.end());
  assert((n % 2));
  write(a[(n / 2)], "\n");
}
