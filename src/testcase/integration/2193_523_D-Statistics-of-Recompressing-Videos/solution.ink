// Translated from solution.cpp.

var q: dynamic = cpp_uninitialized();

func mx(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? a : b;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  ios.sync_with_stdio(false);
  read(n, k);
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      q.push(0);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      var z: dynamic = (mx(x, q.top()) + y);
      write(z, "\n");
      q.pop();
      q.push(z);
      i += 1;
    }
  }
  return 0;
}
