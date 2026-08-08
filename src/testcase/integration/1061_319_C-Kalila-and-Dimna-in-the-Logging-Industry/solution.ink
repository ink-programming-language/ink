// Translated from solution.cpp.

var MAX_N = 100001;

var N: dynamic;

var A = cpp_array(MAX_N);

var B = cpp_array(MAX_N);

var C = cpp_array(MAX_N);

var intervals: dynamic;

var lines: dynamic;

func find_intersection_x(d1: dynamic, d2: dynamic)
{
  var a1 = B[d1];
  var b1 = C[d1];
  var a2 = B[d2];
  var b2 = C[d2];
  return (((b2 - b1)) / ((a1 - a2)));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(N);
  {
    var i = 1;
    while ((i <= N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= N))
    {
      read(B[i]);
      i += 1;
    }
  }
  intervals.push_back(pair(LLONG_MIN, LLONG_MAX));
  lines.push_back(1);
  var s: dynamic;
  s.push(1);
  C[1] = 0;
  {
    var i = 2;
    var id = 0;
    while ((i <= N))
    {
      var di = s.top();
      while (((id < intervals.size()) && (intervals[id].second < A[i])))
      {
        id += 1;
      }
      var lineid = lines[id];
      var a = B[lineid];
      var b = C[lineid];
      C[i] = (b + (a * A[i]));
      var xp = find_intersection_x(di, i);
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
