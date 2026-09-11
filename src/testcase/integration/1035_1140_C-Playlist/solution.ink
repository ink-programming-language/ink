// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var idx: dynamic = 0;
    while ((idx < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      songs[idx].first = b;
      songs[idx].second = a;
      len[idx] = a;
      idx += 1;
    }
  }
  sort(songs.begin(), songs.end());
  var curSum: dynamic = 0;
  var maxPleasure: dynamic = LLONG_MIN;
  var q: dynamic = cpp_uninitialized();
  {
    var idx: dynamic = (n - 1);
    while ((idx >= 0))
    {
      if ((q.size() < k))
      {
        curSum += songs[idx].second;
        q.push(songs[idx].second);
        maxPleasure = max((curSum * songs[idx].first), maxPleasure);
      } else if ((q.size() >= k))
      {
        maxPleasure = max(((((curSum + songs[idx].second) - q.top())) * songs[idx].first), maxPleasure);
        if ((songs[idx].second > q.top()))
        {
          curSum -= q.top();
          q.pop();
          q.push(songs[idx].second);
          curSum += songs[idx].second;
        }
      }
      idx -= 1;
    }
  }
  write(maxPleasure, "\n");
}
