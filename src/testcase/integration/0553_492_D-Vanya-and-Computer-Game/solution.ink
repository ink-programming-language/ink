// Translated from solution.cpp.

var maxn: dynamic = (100000 + 5);

var v: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var name: dynamic = ["Vanya", "Vova", "Both"];

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(n, x, y);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var cntx: dynamic = 0;
  var cnty: dynamic = 0;
  while (((cntx < x) || (cnty < y)))
  {
    if (((cpp_cast(((cntx + 1))) / cpp_cast(x)) > (cpp_cast(((cnty + 1))) / cpp_cast(y))))
    {
      v.push_back(1);
      cnty += 1;
    } else if (((cpp_cast(((cntx + 1))) / cpp_cast(x)) < (cpp_cast(((cnty + 1))) / cpp_cast(y))))
    {
      v.push_back(0);
      cntx += 1;
    } else
    {
      v.push_back(2);
      v.push_back(2);
      cnty += 1;
      cntx += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(name[v[(((a[i] - 1)) % ((x + y)))]], cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
