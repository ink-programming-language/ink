// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var res: dynamic;
  for (var v in vec)
  {
    read(v);
  }
  if ((k == 1))
  {
    return cpp_comma((cout << vec[0]), 0);
  }
  var l = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= i))
        {
          l += 1;
          if ((l == k))
          {
            return cpp_comma((cout << vec[(j - 1)]), 0);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
