// Translated from solution.cpp.

var inp: dynamic;

var diff: dynamic;

var cnt: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(inp);
  diff.push_back(inp[0]);
  cnt.push_back(1);
  {
    var i = 1;
    while ((i < inp.size()))
    {
      if ((inp[i] != inp[(i - 1)]))
      {
        diff.push_back(inp[i]);
        cnt.push_back(1);
      } else
      {
        ((*cnt.rbegin())) += 1;
      }
      i += 1;
    }
  }
  if (((diff.size() % 2) == 0))
  {
    write(0, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < (diff.size() / 2)))
    {
      if ((diff[i] != diff[((diff.size() - 1) - i)]))
      {
        write(0, "\n");
        return 0;
      }
      if (((cnt[i] + cnt[((diff.size() - 1) - i)]) < 3))
      {
        write(0, "\n");
        return 0;
      }
      i += 1;
    }
  }
  if ((cnt[(diff.size() / 2)] >= 2))
  {
    write((cnt[(diff.size() / 2)] + 1), "\n");
  } else
  {
    write(0, "\n");
  }
  return 0;
}
