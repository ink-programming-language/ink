// Translated from solution.cpp.

var MOD = (1e9 + 7);

var inf = 1e9;

var INF = 1e18;

var pi = 3.14159265358979323846;

func main()
{
  var n: dynamic;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var dp = cpp_array(n);
  fill(dp, (dp + n), inf);
  {
    var i = 0;
    while ((i < n))
    {
      (*lower_bound(dp, (dp + n), a[i])) = a[i];
      i += 1;
    }
  }
  write((lower_bound(dp, (dp + n), inf) - dp), "\n");
}
