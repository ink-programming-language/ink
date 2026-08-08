// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(realArr[i]);
      arr[i].first = (realArr[i] % m);
      arr[i].second = i;
      i += 1;
    }
  }
  sort(arr.begin(), arr.end());
  var countStart = 0;
  var countEnd = (n - 1);
  var num = 0;
  var moves = 0;
  while ((num < m))
  {
    var same = (n / m);
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
      var out_i = 1;
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
