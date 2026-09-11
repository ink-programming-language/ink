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

class Rectangle
{
  var h: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
}

func max_area_of_histgram(n: dynamic, height: dynamic) -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  var maxv: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      var rect: dynamic = cpp_uninitialized();
      rect.h = height[i];
      rect.p = i;
      if (S.empty())
      {
        S.push(rect);
      } else if ((S.top().h < rect.h))
      {
        S.push(rect);
      } else if ((S.top().h > rect.h))
      {
        var now: dynamic = i;
        while ((((!S.empty())) && ((S.top().h >= rect.h))))
        {
          var pre: dynamic = S.top();
          S.pop();
          var area: dynamic = ((1 * pre.h) * ((i - pre.p)));
          maxv = max(maxv, area);
          now = pre.p;
        }
        rect.p = now;
        S.push(rect);
      }
      i += 1;
    }
  }
  return maxv;
}

func solve() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var dp: dynamic = cpp_construct(N, 0);
  for (var in_cpp: dynamic in dp)
  {
    read(in_cpp);
  }
  dp.push_back(0);
  return max_area_of_histgram(N, dp);
}

func main(argument_0: dynamic) -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  write(solve(), "\n");
  return 0;
}
