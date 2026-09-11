// Translated from solution.cpp.

func err(it: dynamic) -> dynamic
{
}

func err(it: dynamic, a: dynamic, args: dynamic...) -> dynamic
{
  write((*it), " = ", a, "\n");
  err(cpp_update(it, "++"), cpp_expand(args));
}

var M: dynamic = (1e9 + 7);

var inf: dynamic = 1e9;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      s.insert(a[i]);
      i += 1;
    }
  }
  var x: dynamic = inf;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var it: dynamic = s.lower_bound((k - b[i]));
      if ((it != s.end()))
      {
        x = min(x, (b[i] + (*it)));
      }
      i += 1;
    }
  }
  var beh: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var it: dynamic = s.lower_bound((x - b[i]));
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
