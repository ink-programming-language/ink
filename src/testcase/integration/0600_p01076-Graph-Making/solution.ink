// Translated from solution.cpp.

var INF: dynamic = 1e9;

var LINF: dynamic = 1e18;

func operator_shift_left(out: dynamic, o: dynamic) -> dynamic
{
  (((((out << "(") << o.first) << ",") << o.second) << ")");
  return out;
}

func operator_shift_left(out: dynamic, V: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
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

func operator_shift_left(out: dynamic, Mat: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
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

func operator_shift_left(out: dynamic, mp: dynamic) -> dynamic
{
  (out << "{ ");
  {
    var it: dynamic = mp.begin();
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

func solve() -> dynamic
{
  var res: dynamic = 0;
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
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

func main(argument_0: dynamic) -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  write(solve(), "\n");
  return 0;
}
