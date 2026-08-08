// Translated from solution.cpp.

var N = (3e6 + 7);

var a = cpp_array(N);

var b = cpp_array(N);

var c = cpp_array(N);

var tree = cpp_array(N);

var lazy = cpp_array(N);

var cnt: dynamic;

var sum: dynamic;

var s2: dynamic;

var mx: dynamic;

var v: dynamic;

var u: dynamic;

var vp: dynamic;

var mp: dynamic;

var vis = cpp_array(111111);

var p: dynamic;

var ss: dynamic;

var q: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var m = 0;
  var d: dynamic;
  var i = 0;
  var s1 = 0;
  var s2 = 0;
  var q = 5;
  var x = 0;
  var y: dynamic;
  var k: dynamic;
  var j: dynamic;
  var s = "";
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
