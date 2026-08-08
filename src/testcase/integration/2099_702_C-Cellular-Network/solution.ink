// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var r: dynamic;

var np: dynamic;

var vs: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      var t: dynamic;
      read(t);
      np.insert(t);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var t: dynamic;
      read(t);
      vs.insert(t);
      i += 1;
    }
  }
  {
    var it = np.begin();
    while ((it != np.end()))
    {
      if ((vs.find((*it)) != vs.end()))
      {
        it += 1;
        continue;
      }
      var t = (*it);
      var it1: dynamic;
      if ((((*vs.rbegin()) > t) && ((*vs.begin()) < t)))
      {
        it1 = vs.upper_bound(t);
      } else if ((t > (*vs.rbegin())))
      {
        it1 = vs.end();
        it1 -= 1;
      } else
      {
        r = max(r, abs((t - (*vs.begin()))));
        it += 1;
        continue;
      }
      var v1 = (*it1);
      it1 -= 1;
      var v2 = (*it1);
      r = max(r, min(abs((t - v1)), abs((t - v2))));
      it += 1;
    }
  }
  write(r);
  return 0;
}
