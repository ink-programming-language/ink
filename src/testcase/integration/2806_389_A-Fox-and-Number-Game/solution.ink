// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var i: dynamic;
  var l: dynamic;
  var q = 0;
  var r = 0;
  var m: dynamic;
  var p = 0;
  var z: dynamic;
  var f: dynamic;
  var w = 100;
  var k = 0;
  var j = 0;
  var t = 0;
  read(n);
  var a = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    i = 0;
    while ((i < 100))
    {
      {
        j = (n - 1);
        while ((j > 0))
        {
          if ((a[j] > a[(j - 1)]))
          {
            a[j] -= a[(j - 1)];
          }
          j -= 1;
        }
      }
      sort(a, (a + n));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      k += a[i];
      i += 1;
    }
  }
  write(k);
}
