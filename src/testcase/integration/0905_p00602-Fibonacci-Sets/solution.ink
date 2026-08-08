// Translated from solution.cpp.

var N = cpp_expression("#inc");

var M = cpp_expression("#inc");

var P = cpp_expression("#i");

func main()
{
  var v: dynamic;
  var d: dynamic;
  var f = cpp_array(N);
  var F: dynamic;
  while (((cin >> v) >> d))
  {
    f[0] = cpp_assign(f[1], "=", 1);
    {
      var i = 2;
      while ((i < (2 + v)))
      {
        f[i] = (((f[(i - 1)] + f[(i - 2)])) % M);
        F.push_back(f[i]);
        i += 1;
      }
    }
    sort(F.begin(), F.end());
    var ans = 0;
    {
      var i = 1;
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
