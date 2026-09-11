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
  var N: dynamic = cpp_uninitialized();
  read(N);
  for (var in_cpp: dynamic in a)
  {
    read(in_cpp);
  }
  __cpp_lambda_1();
}

func main(argument_0: dynamic) -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
  return 0;
}

func __cpp_lambda_1() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < N))
        {
          if ((a[i] == a[j]))
          {
            j += 1;
            continue;
          }
          if (((abs((a[i] - a[j])) % ((N - 1))) == 0))
          {
            write(a[i], " ", a[j], "\n");
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return;
}
