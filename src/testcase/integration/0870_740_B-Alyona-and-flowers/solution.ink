// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  for (var i in val)
  {
    read(i);
  }
  var a: dynamic;
  var b: dynamic;
  var res = 0;
  {
    var i = 0;
    while ((i < m))
    {
      read(a, b);
      var t = 0;
      {
        var j = (a - 1);
        while ((j < b))
        {
          t += val[j];
          j += 1;
        }
      }
      if ((t > 0))
      {
        res += t;
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
