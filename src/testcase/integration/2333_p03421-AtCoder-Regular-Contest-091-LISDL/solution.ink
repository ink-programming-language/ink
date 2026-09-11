// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, a, b);
  if (cpp_binary(((a + b) > (n + 1)), "or", ((a * cpp_cast(b)) < n)))
  {
    write("-1");
    return 0;
  }
  var v: dynamic = cpp_construct((a - 1), 0);
  {
    var i: dynamic = 1;
    while ((i <= (n - b)))
    {
      v[(i % v.size())] += 1;
      i += 1;
    }
  }
  v.push_back(b);
  var k: dynamic = 1;
  for (var i: dynamic in v)
  {
    var l: dynamic = ((k + i) - 1);
    while ((l >= k))
    {
      write(cpp_update(l, "--"), cpp_char(" "));
    }
    k += i;
  }
}
