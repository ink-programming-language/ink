// Translated from solution.cpp.

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var ans: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      sum += x;
      v.push_back(make_pair(x, sum));
      i += 1;
    }
  }
  if (((sum % 3) == 0))
  {
    var a: dynamic = (sum / 3);
    var b: dynamic = (a * 2);
    x = 0;
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        if ((v[i].second == b))
        {
          ans += x;
        }
        if ((v[i].second == a))
        {
          x += 1;
        }
        i += 1;
      }
    }
  }
  write(ans, "\n");
  return 0;
}
