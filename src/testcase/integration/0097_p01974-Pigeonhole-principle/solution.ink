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
  var N: dynamic;
  read(N);
  for (var in_cpp in a)
  {
    read(in_cpp);
  }
  __cpp_lambda_1();
}

func main(argument_0: dynamic)
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
  return 0;
}

func __cpp_lambda_1()
{
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = (i + 1);
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
