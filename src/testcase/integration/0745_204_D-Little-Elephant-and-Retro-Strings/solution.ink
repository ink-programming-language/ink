// Translated from solution.cpp.

func dbs(str: dynamic, t: dynamic)
{
  write(str, " : ", t, "\n");
}

func dbs(str: dynamic, t: dynamic, s: dynamic...)
{
  var idx = str.find(cpp_char(","));
  write(str.substr(0, idx), " : ", t, ",");
  dbs(str.substr((idx + 1)), cpp_expand(s));
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (os << "[ ");
  for (var it in p)
  {
    ((os << it) << " ");
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (os << "[ ");
  for (var it in p)
  {
    ((os << it) << " ");
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (os << "[ ");
  for (var it in p)
  {
    ((os << it) << " ");
  }
  return (os << "]");
}

func prc(a: dynamic, b: dynamic)
{
  write("[");
  {
    var i = a;
    while ((i != b))
    {
      if ((i != a))
      {
        write(", ");
      }
      write((*i));
      i += 1;
    }
  }
  write("]\n");
}

var N = 1000010;

var BSum = cpp_array((N + 1));

var WSum = cpp_array((N + 1));

var leftmost = cpp_array((N + 1));

var Bfree = cpp_array((N + 1));

var rightmost = cpp_array((N + 1));

var Wfree = cpp_array((N + 1));

var s: dynamic;

func computeLeft(n: dynamic, k: dynamic)
{
  Bfree[0] = 1;
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if (cpp_binary((i >= k), "and", ((WSum[i] - WSum[(i - k)]) == 0)))
      {
        if (cpp_binary((i == k), "or", (s[((i - k) - 1)] != cpp_char("B"))))
        {
          leftmost[i] = Bfree[max(((i - k) - 1), 0)];
        }
      }
      var temp = Bfree[(i - 1)];
      if ((s[(i - 1)] == cpp_char("X")))
      {
        temp = (((temp * 2)) % 1000000007);
      }
      temp = ((((temp - leftmost[i]) + 1000000007)) % 1000000007);
      Bfree[i] = temp;
      i += 1;
    }
  }
}

func computeRight(n: dynamic, k: dynamic)
{
  Wfree[n] = 1;
  {
    var i = cpp_cast(((n - 1)));
    while ((i >= cpp_cast((0))))
    {
      if (cpp_binary((i <= (n - k)), "and", ((BSum[(i + k)] - BSum[i]) == 0)))
      {
        if (cpp_binary((i == (n - k)), "or", (s[(i + k)] != cpp_char("W"))))
        {
          rightmost[i] = Wfree[min(((i + k) + 1), n)];
        }
      }
      var temp = Wfree[(i + 1)];
      if ((s[i] == cpp_char("X")))
      {
        temp = (((temp * 2)) % 1000000007);
      }
      temp = ((((temp - rightmost[i]) + 1000000007)) % 1000000007);
      Wfree[i] = temp;
      i -= 1;
    }
  }
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  read(s);
  {
    var i = cpp_cast((0));
    while ((i <= cpp_cast(((n - 1)))))
    {
      BSum[(i + 1)] = (BSum[i] + (if ((s[i] == cpp_char("B"))) 1 else 0));
      WSum[(i + 1)] = (WSum[i] + (if ((s[i] == cpp_char("W"))) 1 else 0));
      i += 1;
    }
  }
  computeLeft(n, k);
  computeRight(n, k);
  var answer1 = 0;
  var twoPow = 1;
  {
    var i = cpp_cast(((n - 1)));
    while ((i >= cpp_cast((0))))
    {
      if ((s[i] == cpp_char("X")))
      {
        twoPow = (((twoPow * 2)) % 1000000007);
      }
      var containW = ((((twoPow - Wfree[i]) + 1000000007)) % 1000000007);
      answer1 = (((answer1 + (leftmost[i] * containW))) % 1000000007);
      i -= 1;
    }
  }
  write(answer1, "\n");
  return 0;
}
