// Translated from solution.cpp.

var dr: dynamic = [1, 0, -1, 0, 1, 1, -1, -1];

var dc: dynamic = [0, 1, 0, -1, 1, -1, -1, 1];

var eps: dynamic = 1e-9;

var INF: dynamic = 0x7FFFFFFF;

var INFLL: dynamic = 0x7FFFFFFFFFFFFFFF;

var pi: dynamic = acos(-1);

func take(O: dynamic) -> dynamic
{
  var tmp: dynamic = O.front();
  O.pop();
  return tmp;
}

func take(O: dynamic) -> dynamic
{
  var tmp: dynamic = O.top();
  O.pop();
  return tmp;
}

func take(O: dynamic) -> dynamic
{
  var tmp: dynamic = O.top();
  O.pop();
  return tmp;
}

func inRange(z: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return ((a <= z) && (z <= b));
}

func OPEN(in_cpp: dynamic = "input.txt", out: dynamic = "output.txt") -> dynamic
{
  freopen(in_cpp.c_str(), "r", stdin);
  freopen(out.c_str(), "w", stdout);
  return;
}

var PQ: dynamic = cpp_array(1000005);

var caw: dynamic = cpp_array(1000005);

var ans: dynamic = INFLL;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

class Data
{
  var d: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var idx: dynamic = cpp_uninitialized();
}

var data: dynamic = cpp_array(1000005);

var dvec: dynamic = cpp_array(1000005);

var adacnt: dynamic = 0;

var ada: dynamic = cpp_uninitialized();

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, m, k);
  {
    int_cpp(i) = (0);
    t = (m);
    while ((i < (t)))
    {
      read(data[i]);
      dvec[data[i].d].push_back(i);
      data[i].idx = i;
      (i) += 1;
    }
  }
  {
    int_cpp(z) = (1000000);
    t = ((k + 1));
    while ((z >= (t)))
    {
      for (var id: dynamic in dvec[z])
      {
        var d: dynamic = data[id];
        if ((d.f == 0))
        {
          PQ[d.t].push(make_pair((-d.c), d.idx));
        }
      }
      (z) -= 1;
    }
  }
  var tmpAns: dynamic = 0;
  {
    int_cpp(i) = (1);
    t = (n);
    while ((i <= (t)))
    {
      if (PQ[i].empty())
      {
        write("-1\n");
        return 0;
      }
      tmpAns += (-PQ[i].top().first);
      (i) += 1;
    }
  }
  var ed: dynamic = (k + 1);
  {
    int_cpp(st) = (1);
    t = (((1000000 - k) + 1));
    while ((st <= (t)))
    {
      for (var id: dynamic in dvec[st])
      {
        var d: dynamic = data[id];
        if ((d.t == 0))
        {
          tmpAns -= caw[d.f];
          if ((!ada.test(d.f)))
          {
            adacnt += 1;
            caw[d.f] = INF;
          }
          ada.set(d.f, 1);
          caw[d.f] = min(caw[d.f], d.c);
          tmpAns += caw[d.f];
        }
      }
      var abis: dynamic = 0;
      for (var id: dynamic in dvec[ed])
      {
        var d: dynamic = data[id];
        if ((d.f == 0))
        {
          var PQ: dynamic = PQ[d.t];
          tmpAns -= (-PQ.top().first);
          while (((!PQ.empty()) && (data[PQ.top().second].d <= ed)))
          {
            PQ.pop();
          }
          if (PQ.empty())
          {
            abis = 1;
            break;
          }
          tmpAns += (-PQ.top().first);
        }
      }
      ed += 1;
      if (abis)
      {
        break;
      }
      if ((adacnt == n))
      {
        ans = min(ans, tmpAns);
      }
      (st) += 1;
    }
  }
  if ((ans == INFLL))
  {
    write(-1, "\n");
    return 0;
  }
  write(ans, "\n");
}
