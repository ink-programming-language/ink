// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var q: dynamic;

var l: dynamic;

var r: dynamic;

var s: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  read(l, r);
  a = (r - l);
  if (((r - l) < 100))
  {
    var mp: dynamic;
    var it: dynamic;
    {
      var i = l;
      while ((i <= r))
      {
        a = i;
        {
          var x = 2;
          while (((x * x) <= a))
          {
            if (((a % x) == 0))
            {
              mp[(a / x)] += 1;
              mp[x] += 1;
            }
            x += 1;
          }
        }
        if ((a > 1))
        {
          mp[a] += 1;
        }
        i += 1;
      }
    }
    {
      it = mp.begin();
      while ((it != mp.end()))
      {
        b = max(b, it->second);
        it += 1;
      }
    }
    {
      it = mp.begin();
      while ((it != mp.end()))
      {
        if ((it->second == b))
        {
          write(it->first);
          return 0;
        }
        it += 1;
      }
    }
  } else
  {
    write(2);
  }
}
