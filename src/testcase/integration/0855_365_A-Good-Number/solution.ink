// Translated from solution.cpp.

var b: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var sum2: dynamic = cpp_uninitialized();

func num_to_string(num: dynamic) -> dynamic
{
  var ss: dynamic = cpp_uninitialized();
  (ss << num);
  return ss.str();
}

func O_o() -> dynamic
{
  ios.sync_with_stdio(0);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func main() -> dynamic
{
  O_o();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s);
      var ss: dynamic = cpp_uninitialized();
      b = 0;
      {
        var j: dynamic = 0;
        while ((j < (cpp_cast((s.size())))))
        {
          ss.insert((s[j] - cpp_char("0")));
          j += 1;
        }
      }
      var x: dynamic = 0;
      for (var j: dynamic in ss)
      {
        if ((j != x))
        {
          b = 1;
        }
        if ((x == k))
        {
          break;
        }
        x += 1;
      }
      if ((((cpp_cast((ss.size()))) < (k + 1)) || (x != k)))
      {
        b = 1;
      }
      if ((!b))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
