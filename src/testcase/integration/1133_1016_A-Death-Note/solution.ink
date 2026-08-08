// Translated from solution.cpp.

var a = cpp_array(200005);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie();
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      a[i] = (a[(i - 1)] + x);
      i += 1;
    }
  }
  var tmp = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      write(((a[i] / m) - tmp), " ");
      tmp += (((a[i] / m) - tmp));
      i += 1;
    }
  }
}
