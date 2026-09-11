// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var np: dynamic = cpp_uninitialized();

var vs: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var t: dynamic = cpp_uninitialized();
      read(t);
      np.insert(t);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var t: dynamic = cpp_uninitialized();
      read(t);
      vs.insert(t);
      i += 1;
    }
  }
  {
    var it: dynamic = np.begin();
    while ((it != np.end()))
    {
      if ((vs.find((*it)) != vs.end()))
      {
        it += 1;
        continue;
      }
      var t: dynamic = (*it);
      var it1: dynamic = cpp_uninitialized();
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
      var v1: dynamic = (*it1);
      it1 -= 1;
      var v2: dynamic = (*it1);
      r = max(r, min(abs((t - v1)), abs((t - v2))));
      it += 1;
    }
  }
  write(r);
  return 0;
}
