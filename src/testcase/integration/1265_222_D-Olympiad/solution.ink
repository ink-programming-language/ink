// Translated from solution.cpp.

func err(it: dynamic)
{
}

func err(it: dynamic, a: dynamic, args: dynamic...)
{
  write((*it), " = ", a, "\n");
  err(cpp_update(it, "++"), cpp_expand(args));
}

var M = (1e9 + 7);

var inf = 1e9;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array(n);
  var b = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      s.insert(a[i]);
      i += 1;
    }
  }
  var x = inf;
  {
    var i = 0;
    while ((i < n))
    {
      var it = s.lower_bound((k - b[i]));
      if ((it != s.end()))
      {
        x = min(x, (b[i] + (*it)));
      }
      i += 1;
    }
  }
  var beh = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var it = s.lower_bound((x - b[i]));
      if ((it != s.end()))
      {
        s.erase(it);
      } else
      {
        beh += 1;
      }
      i += 1;
    }
  }
  write(1, " ", (n - beh));
}
