// Translated from solution.cpp.

var n: dynamic;

var a: dynamic;

var b: dynamic;

func main()
{
  read(n, a, b);
  if (cpp_binary(((a + b) > (n + 1)), "or", ((a * cpp_cast(b)) < n)))
  {
    write("-1");
    return 0;
  }
  var v = cpp_construct((a - 1), 0);
  {
    var i = 1;
    while ((i <= (n - b)))
    {
      v[(i % v.size())] += 1;
      i += 1;
    }
  }
  v.push_back(b);
  var k = 1;
  for (var i in v)
  {
    var l = ((k + i) - 1);
    while ((l >= k))
    {
      write(cpp_update(l, "--"), cpp_char(" "));
    }
    k += i;
  }
}
