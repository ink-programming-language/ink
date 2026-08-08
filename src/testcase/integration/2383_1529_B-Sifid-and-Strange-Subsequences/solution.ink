// Translated from solution.cpp.

var ll = dynamic;

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var vii = cpp_expression("#include<b");

var pii = cpp_expression("#include<bi");

var mii = cpp_expression("#include<b");

func fastio()
{
  cpp_macro("ios::sync_with_stdio(0);cin.tie(0);cout.tie(0);");
}

func main()
{
  fastio();
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var a = cpp_array(n);
    for (var x in a)
    {
      read(x);
    }
    sort(a, (a + n));
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if (((a[(i + 1)] - a[i]) < a[(n - 1)]))
        {
          n -= 1;
          i -= 1;
        }
        i += 1;
      }
    }
    write(n, "\n");
  }
  return 0;
}
