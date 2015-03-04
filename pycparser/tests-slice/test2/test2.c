int copy(int a) {
  return a;
}

int main() {
  int a[] = {1, 1};

  a[0]++;
  a[1]++;
  a[0] += 1;
  a[1] += 1;
  a[0] = a[0] + 2;
  a[1] = a[1] + 2;

  switch (a[0]) {
  case 0:
    a[0]++;
    a[0] = copy(a[0]);
    a[1] = copy(a[1]);
    copy(a[1]);
    break;
  default:
    break;
  }
      
}
