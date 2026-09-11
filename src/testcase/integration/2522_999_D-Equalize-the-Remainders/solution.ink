// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(realArr[i]);
      arr[i].first = (realArr[i] % m);
      arr[i].second = i;
      i += 1;
    }
  }
  sort(arr.begin(), arr.end());
  var countStart: dynamic = 0;
  var countEnd: dynamic = (n - 1);
  var num: dynamic = 0;
  var moves: dynamic = 0;
  while ((num < m))
  {
    var same: dynamic = (n / m);
    while (cpp_update(same, "--"))
    {
      if ((arr[countStart].first > num))
      {
        realArr[arr[countEnd].second] += ((m - arr[countEnd].first) + num);
        moves += ((m - arr[countEnd].first) + num);
        countEnd -= 1;
      } else
      {
        realArr[arr[countStart].second] += (num - arr[countStart].first);
        moves += (num - arr[countStart].first);
        countStart += 1;
      }
    }
    num += 1;
  }
  write(moves, "\n");
  if ((realArr).empty())
  {
    write("\n");
  } else
  {
    write((realArr)[0]);
    {
      var out_i: dynamic = 1;
      while ((out_i < (realArr).size()))
      {
        write(" ", (realArr)[out_i]);
        out_i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
