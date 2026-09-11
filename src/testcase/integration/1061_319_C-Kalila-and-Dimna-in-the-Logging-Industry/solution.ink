// Translated from solution.cpp.

var MAX_N: dynamic = 100001;

var N: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(MAX_N);

var B: dynamic = cpp_array(MAX_N);

var C: dynamic = cpp_array(MAX_N);

var intervals: dynamic = cpp_uninitialized();

var lines: dynamic = cpp_uninitialized();

func find_intersection_x(d1: dynamic, d2: dynamic) -> dynamic
{
  var a1: dynamic = B[d1];
  var b1: dynamic = C[d1];
  var a2: dynamic = B[d2];
  var b2: dynamic = C[d2];
  return (((b2 - b1)) / ((a1 - a2)));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(N);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(B[i]);
      i += 1;
    }
  }
  intervals.push_back(pair(LLONG_MIN, LLONG_MAX));
  lines.push_back(1);
  var s: dynamic = cpp_uninitialized();
  s.push(1);
  C[1] = 0;
  {
    var i: dynamic = 2;
    var id: dynamic = 0;
    while ((i <= N))
    {
      var di: dynamic = s.top();
      while (((id < intervals.size()) && (intervals[id].second < A[i])))
      {
        id += 1;
      }
      var lineid: dynamic = lines[id];
      var a: dynamic = B[lineid];
      var b: dynamic = C[lineid];
      C[i] = (b + (a * A[i]));
      var xp: dynamic = find_intersection_x(di, i);
      while (((!intervals.empty()) && (xp < intervals.back().first)))
      {
        s.pop();
        lines.pop_back();
        intervals.pop_back();
        di = s.top();
        xp = find_intersection_x(di, i);
      }
      s.push(i);
      lines.push_back(i);
      intervals.back().second = xp;
      intervals.push_back(pair(xp, LLONG_MAX));
      i += 1;
    }
  }
  write(C[N]);
  return 0;
}
