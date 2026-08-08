// Translated from solution.cpp.

var q: dynamic;

func mx(a: dynamic, b: dynamic)
{
  return if ((a > b)) a else b;
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  ios.sync_with_stdio(false);
  read(n, k);
  {
    var i = 1;
    while ((i <= k))
    {
      q.push(0);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      var z = (mx(x, q.top()) + y);
      write(z, "\n");
      q.pop();
      q.push(z);
      i += 1;
    }
  }
  return 0;
}
