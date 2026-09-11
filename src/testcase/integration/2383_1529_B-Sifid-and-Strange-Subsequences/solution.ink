// Translated from solution.cpp.

var ll: dynamic = dynamic;

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var vii: dynamic = cpp_expression("#include<b");

var pii: dynamic = cpp_expression("#include<bi");

var mii: dynamic = cpp_expression("#include<b");

func fastio() -> dynamic
{
  cpp_macro("ios::sync_with_stdio(0);cin.tie(0);cout.tie(0);");
}

func main() -> dynamic
{
  fastio();
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var a: dynamic = cpp_array(n);
    for (var x: dynamic in a)
    {
      read(x);
    }
    sort(a, (a + n));
    {
      var i: dynamic = 0;
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
