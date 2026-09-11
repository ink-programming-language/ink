// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var q: dynamic = 0;
  var r: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var p: dynamic = 0;
  var z: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var w: dynamic = 100;
  var k: dynamic = 0;
  var j: dynamic = 0;
  var t: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
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
