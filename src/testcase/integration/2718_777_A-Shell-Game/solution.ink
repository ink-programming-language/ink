// Translated from solution.cpp.

var N: dynamic = (3e6 + 7);

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var tree: dynamic = cpp_array(N);

var lazy: dynamic = cpp_array(N);

var cnt: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var vp: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(111111);

var p: dynamic = cpp_uninitialized();

var ss: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = 0;
  var d: dynamic = cpp_uninitialized();
  var i: dynamic = 0;
  var s1: dynamic = 0;
  var s2: dynamic = 0;
  var q: dynamic = 5;
  var x: dynamic = 0;
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var s: dynamic = "";
  read(n, x);
  if ((x == 0))
  {
    a[0] = 1;
  }
  if ((x == 1))
  {
    a[1] = 1;
  }
  if ((x == 2))
  {
    a[2] = 1;
  }
  m = (n % 6);
  if (((n % 2) != 0))
  {
    {
      i = 1;
      while ((i <= m))
      {
        if (((i % 2) != 0))
        {
          swap(a[0], a[1]);
        } else
        {
          swap(a[1], a[2]);
        }
        i += 1;
      }
    }
    if ((a[0] == 1))
    {
      write(0);
    }
    if ((a[1] == 1))
    {
      write(1);
    }
    if ((a[2] == 1))
    {
      write(2);
    }
    return 0;
  } else
  {
    {
      i = 1;
      while ((i <= m))
      {
        if (((i % 2) == 0))
        {
          swap(a[0], a[1]);
        } else
        {
          swap(a[1], a[2]);
        }
        i += 1;
      }
    }
    if ((a[0] == 1))
    {
      write(0);
    }
    if ((a[1] == 1))
    {
      write(1);
    }
    if ((a[2] == 1))
    {
      write(2);
    }
    return 0;
  }
}
