// Translated from solution.cpp.

var a: dynamic = cpp_array(200005);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie();
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      a[i] = (a[(i - 1)] + x);
      i += 1;
    }
  }
  var tmp: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(((a[i] / m) - tmp), " ");
      tmp += (((a[i] / m) - tmp));
      i += 1;
    }
  }
}
