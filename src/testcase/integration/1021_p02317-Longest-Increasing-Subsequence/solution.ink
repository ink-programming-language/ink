// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var inf: dynamic = 1e9;

var INF: dynamic = 1e18;

var pi: dynamic = 3.14159265358979323846;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var dp: dynamic = cpp_array(n);
  fill(dp, (dp + n), inf);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      (*lower_bound(dp, (dp + n), a[i])) = a[i];
      i += 1;
    }
  }
  write((lower_bound(dp, (dp + n), inf) - dp), "\n");
}
