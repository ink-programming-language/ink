// Translated from solution.cpp.

var N: dynamic = cpp_expression("#inc");

var M: dynamic = cpp_expression("#inc");

var P: dynamic = cpp_expression("#i");

func main() -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_array(N);
  var F: dynamic = cpp_uninitialized();
  while (((cin >> v) >> d))
  {
    f[0] = cpp_assign(f[1], "=", 1);
    {
      var i: dynamic = 2;
      while ((i < (2 + v)))
      {
        f[i] = (((f[(i - 1)] + f[(i - 2)])) % M);
        F.push_back(f[i]);
        i += 1;
      }
    }
    sort(F.begin(), F.end());
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < F.size()))
      {
        if ((abs((F[i] - F[(i - 1)])) >= d))
        {
          ans += 1;
        }
        i += 1;
      }
    }
    write((ans + 1), "\n");
    F.clear();
  }
  return 0;
}
