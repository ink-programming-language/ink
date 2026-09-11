// Translated from solution.cpp.

var X: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

func next() -> dynamic
{
  var y: dynamic = ((((X * A) + B)) % C);
  X = y;
  return X;
}

var MAXN: dynamic = 100;

var names: dynamic = cpp_array(MAXN, MAXN);

var scores: dynamic = cpp_array(MAXN);

var ar: dynamic = cpp_array(MAXN);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (scores[a] > scores[b]);
}

var bask1: dynamic = cpp_uninitialized();

var bask2: dynamic = cpp_uninitialized();

var bask3: dynamic = cpp_uninitialized();

var bask4: dynamic = cpp_uninitialized();

func rm(v: dynamic, i: dynamic) -> dynamic
{
  var tmp: dynamic = v[i];
  v.erase((v.begin() + i));
  return tmp;
}

var N: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  if (fopen("input.txt", "r"))
  {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
  }
  scanf("%d", (&N));
  s = (N / 4);
  scanf("%d %d %d %d", (&X), (&A), (&B), (&C));
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      scanf("%s", names[i]);
      scanf("%d", (scores + i));
      ar[i] = i;
      i += 1;
    }
  }
  sort(ar, (ar + N), cmp);
  {
    var i: dynamic = 0;
    while ((i < s))
    {
      bask1.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = s;
    while ((i < (2 * s)))
    {
      bask2.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (2 * s);
    while ((i < (3 * s)))
    {
      bask3.push_back(ar[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (3 * s);
    while ((i < (4 * s)))
    {
      bask4.push_back(ar[i]);
      i += 1;
    }
  }
  var groupC: dynamic = cpp_char("A");
  while (s)
  {
    var r1: dynamic = rm(bask1, (next() % s));
    var r2: dynamic = rm(bask2, (next() % s));
    var r3: dynamic = rm(bask3, (next() % s));
    var r4: dynamic = rm(bask4, (next() % s));
    s -= 1;
    printf("Group %c:\n%s\n%s\n%s\n%s\n", cpp_update(groupC, "++"), names[r1], names[r2], names[r3], names[r4]);
  }
  return 0;
}
