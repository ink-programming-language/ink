// Translated from solution.cpp.

var INF = 1e9;

var LINF = 1e18;

func operator_shift_left(out: dynamic, o: dynamic)
{
  (((((out << "(") << o.first) << ",") << o.second) << ")");
  return out;
}

func operator_shift_left(out: dynamic, V: dynamic)
{
  {
    var i = 0;
    while ((i < V.size()))
    {
      (out << V[i]);
      if ((i != (V.size() - 1)))
      {
        (out << " ");
      }
      i += 1;
    }
  }
  return out;
}

func operator_shift_left(out: dynamic, Mat: dynamic)
{
  {
    var i = 0;
    while ((i < Mat.size()))
    {
      if ((i != 0))
      {
        (out << endl);
      }
      (out << Mat[i]);
      i += 1;
    }
  }
  return out;
}

func operator_shift_left(out: dynamic, mp: dynamic)
{
  (out << "{ ");
  {
    var it = mp.begin();
    while ((it != mp.end()))
    {
      (((out << it->first) << ":") << it->second);
      if (((mp.size() - 1) != distance(mp.begin(), it)))
      {
        (out << ", ");
      }
      it += 1;
    }
  }
  (out << " }");
  return out;
}

func solve()
{
  var res = 0;
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  if ((d == 1))
  {
    return ((n * ((n - 1))) / 2);
  }
  res += (d - 1);
  res += ((((n - d)) * (((n - d) + 1))) / 2);
  res += (((n - d) - 1));
  return res;
}

func main(argument_0: dynamic)
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  write(solve(), "\n");
  return 0;
}
